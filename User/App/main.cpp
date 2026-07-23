/**
 * @file main.cpp
 * @brief FreeRTOS — 菜单 + 传感器显示 + 电机控制
 *
 * 任务架构:
 *   Ctrl  (1ms,  Prio 3) — 编码器 + 电机控制 + 灰度 + CAN + VL53L0X + 双陀螺仪 + INS
 *   Disp  (50ms, Prio 2) — 按键扫描 + 菜单渲染 / 传感器显示
 */

#include "main.hpp"
#include "FreeRTOS.h"
#include "task.h"

#include "motor_control.hpp"
#include "can_protocol.hpp"
#include "bsp_cfg.h"
#include "bsp_tracking.h"
#include "bsp_gyro.h"
#include "bsp_imu601.h"
#include "vl53l0x_sensor.h"
#include "stepper_motor.h"
#include "menu_ui.h"
#include "ins.h"
#include "ssd1306.h"
#include "bsp_gpio.h"

extern Tracking  tracking;
extern Vl53l0x   vl53l0x;

/* ================================================================
 *  菜单定义
 * ================================================================ */
static MenuEntry g_menu[] = {
    { "Sensor View",  "VL+Gray+INS" },
    { "Motor Test",   "Speed/GOTO"  },
    { "Gyro View",    "WT901+IMU601" },
};
static const int g_menu_count = sizeof(g_menu) / sizeof(g_menu[0]);

/* ---- 全局对象 ---- */
Mt6816        mt6816;
StepperMotor  stepper_motor;
MotorControl  g_motor;
Ssd1306       ssd1306;
Ins           ins;
Menu          g_menu_state;
MenuUI        g_menu_ui;

/* ================================================================
 *  VL53L0X 日志输出 — 覆盖弱函数，接入 bsp_uart_printf
 * ================================================================ */
void vl53l0x_log(const char *msg)
{
    bsp_uart_printf("%s\r\n", msg);
}

/* ================================================================
 *  任务句柄
 * ================================================================ */
static TaskHandle_t g_ctrl_task;
static TaskHandle_t g_disp_task;

/* ================================================================
 *  裸机残留 ISR — FreeRTOS 用 SysTick，清除 TIMA0 中断标志
 * ================================================================ */
extern "C" void TIMA0_IRQHandler(void)
{
    DL_TimerA_clearInterruptStatus(TIMER_TICK_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);
}

/* ================================================================
 *  Ctrl 任务 — 1ms 周期 (最高优先级)
 *
 *  编码器 + 速度计算 + 电机控制(每5ms) + 灰度 + CAN + VL53L0X 状态机
 *  所有操作均为非阻塞，确保 1ms 周期稳定
 * ================================================================ */
static void vControlTask(void *pvParams)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        /* 编码器 + 速度估算 */
        MT6816_Data enc;
        mt6816.read(&enc);
        g_motor.measureTick(&enc);

        /* 灰度循迹（~10μs） */
        tracking.poll();

        /* CAN 收帧 → 状态缓存 */
        can_proto.tick();

        /* 双陀螺仪数据采集 */
        bsp_gyro.poll();
        bsp_imu601.poll();

        /* VL53L0X 测距状态机（非阻塞） */
        vl53l0x.tick();

        /* INS 惯导 */
        ins.tick(0.001f);

        /* 电机控制每 5ms（PWM 计算 + GOTO 闭环） */
        static int tick = 0;
        if (++tick >= 5)
        {
            tick = 0;
            g_motor.controlTick(&enc);
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

/* ================================================================
 *  Disp 任务 — 50ms 周期
 *  按键扫描 + 菜单渲染 / 传感器数据显示
 * ================================================================ */
static void vDisplayTask(void *pvParams)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        g_menu_ui.poll();

        if (g_menu_ui.inTask())
        {
            /* ---- 任务运行模式：根据菜单项显示不同内容 ---- */
            int sel = g_menu_ui.selected();
            if (sel == 0) {  /* Sensor View */
                uint16_t dist  = vl53l0x.get();
                uint8_t  track = tracking.get();
                float    h = ins.heading();
                float    sx = ins.x();
                float    sy = ins.y();

                ssd1306.printf(0, 0, "D:%u T:%02X", (unsigned)dist, (unsigned)track);
                ssd1306.printf(0, 2, "H:%d S:%d", (int)h, (int)ins.speedMmS());
                ssd1306.printf(0, 4, "X:%d Y:%d", (int)sx, (int)sy);
                ssd1306.printf(0, 6, "OK: Back");
            }
            else if (sel == 1) {  /* Motor Test */
                ssd1306.printf(0, 0, "Motor Test");
                ssd1306.printf(0, 2, "Spd:%.0f RPM", g_motor.speed());
                ssd1306.printf(0, 4, "Ang:%.0f Tr:%d", g_motor.angle(), g_motor.turns());
                ssd1306.printf(0, 6, "OK: Back");
            }
            else if (sel == 2) {  /* Gyro View */
                ssd1306.printf(0, 0, "WT Y%3d W%3d",
                    (int)bsp_gyro.getYaw(), (int)bsp_gyro.getWz());
                ssd1306.printf(0, 2, "IM Y%3d P%3d",
                    (int)bsp_imu601.getYaw(), (int)bsp_imu601.getPitch());
                ssd1306.printf(0, 4, "   R%3d", (int)bsp_imu601.getRoll());
                ssd1306.printf(0, 6, "OK: Back");
            }
        }
        else
        {
            g_menu_ui.render();
        }

        bsp_led_toggle();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}

/* ================================================================
 *  主函数
 * ================================================================ */
int main()
{
    SYSCFG_DL_init();

    /* 一键初始化所有外设 + 传感器 */
    bsp.init();
    bsp_uart_printf("=== FreeRTOS ===\r\n");

    /* 设备层 */
    ssd1306.init({&bsp.i2c});
    ssd1306.printf(0, 0, "FreeRTOS OK");
    ssd1306.printf(0, 2, "Booting...");

    mt6816.init({&bsp.spi});
    stepper_motor.init({&bsp.pwm});
    g_motor.init();
    can_proto.init();

    ins.init();
    ins.useWt901();

    /* 菜单初始化 */
    g_menu_state.init(g_menu, g_menu_count);
    g_menu_ui.init(&g_menu_state);

    /* 创建任务：Ctrl(3) > Disp(2) > Idle(0) */
    xTaskCreate(vControlTask, "Ctrl", 256, NULL, 3, &g_ctrl_task);
    xTaskCreate(vDisplayTask, "Disp", 512, NULL, 2, &g_disp_task);

    vTaskStartScheduler();

    /* 不会运行到这里 */
    while (1) {}
}

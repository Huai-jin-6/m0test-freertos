/**
 * @file bsp_cfg.cpp
 * @brief BSP 全局配置 — 所有外设初始化
 */

#include "bsp_cfg.h"
#include "bsp_gyro.h"
#include "bsp_imu601.h"
#include "bsp_tracking.h"
#include "bsp_vl53l0x_i2c.h"
#include "vl53l0x_sensor.h"

/* ---- 全局实例 ---- */
BspCfg    bsp;
Tracking  tracking;
Vl53l0x   vl53l0x;

void BspCfg::init()
{
    spi.init();
    i2c.init();
    pwm.init();
    can.init();
    uart.init();

    /* 传感器 */
    bsp_gyro.init();
    bsp_imu601.init();
    tracking.init();
    vl53l0x_i2c.init();    /* I2C 先初始化，再初始化 VL53L0X */
    vl53l0x.init();
}

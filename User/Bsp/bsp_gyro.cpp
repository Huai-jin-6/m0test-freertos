/**
 * @file bsp_gyro.cpp
 * @brief WT901/JY901 陀螺仪 — UART1 ISR + 环形缓冲 + 协议解析
 *
 * 协议: 0x5A [Type] [DataL] [DataH] [Checksum]
 *   Type=0xAA → Z轴角速度 wz, Type=0xBB → Yaw角度
 */

#include "bsp_gyro.h"
#include "bsp_uart.h"
#include "ti_msp_dl_config.h"

/* ---- 全局实例 + ISR ---- */
BspGyro bsp_gyro;

extern "C" void UART1_IRQHandler(void)
{
    bsp_gyro.isrHandler();
}

/* ---- 环形缓冲 ---- */
void BspGyro::push_(uint8_t b)
{
    uint8_t next = (rxHead_ + 1) & 0x7F;
    if (next == rxTail_) return;
    rxBuf_[rxHead_] = b;
    rxHead_ = next;
}

int BspGyro::pop_(uint8_t *b)
{
    if (rxHead_ == rxTail_) return 0;
    *b = rxBuf_[rxTail_];
    rxTail_ = (rxTail_ + 1) & 0x7F;
    return 1;
}

/* ---- ISR ---- */
void BspGyro::isrHandler()
{
    switch (DL_UART_getPendingInterrupt(UART_Tly_INST))
    {
        case DL_UART_IIDX_RX:
        {
            int limit = 32;
            while (limit-- > 0 && !DL_UART_isRXFIFOEmpty(UART_Tly_INST))
            {
                uint8_t byte = (uint8_t)DL_UART_receiveData(UART_Tly_INST);
                rxTotal_++;
                push_(byte);
            }
            break;
        }
        default: break;
    }
}

/* ---- 协议解析（合并 gyro device 层）---- */
void BspGyro::parse_(uint8_t ucData)
{
    static uint8_t buf[5];
    static uint8_t cnt = 0;

    buf[cnt++] = ucData;

    if (buf[0] != 0x5A) { cnt = 0; return; }
    if (cnt < 5) return;

    uint8_t sum = buf[0] + buf[1] + buf[2] + buf[3];
    if (sum != buf[4]) { cnt = 0; return; }

    short raw = (short)((buf[3] << 8) | buf[2]);

    if      (buf[1] == 0xAA) wz_  = (float)raw / 32768.0f * 2000.0f;
    else if (buf[1] == 0xBB) yaw_ = (float)raw / 32768.0f * 180.0f;

    cnt = 0;
}

/* ---- 公开方法 ---- */

void BspGyro::init()
{
    /* 环回自检 */
    bsp_uart_printf("[GYRO] Loopback test...\r\n");
    NVIC_DisableIRQ(UART_Tly_INST_INT_IRQN);
    DL_UART_enableLoopbackMode(UART_Tly_INST);
    DL_UART_transmitData(UART_Tly_INST, 0xA5);
    while (DL_UART_isBusy(UART_Tly_INST)) {}

    int ok = 0;
    for (int i = 0; i < 2000; i++)
    {
        if (!DL_UART_isRXFIFOEmpty(UART_Tly_INST))
        {
            ok = ((uint8_t)DL_UART_receiveData(UART_Tly_INST) == 0xA5);
            break;
        }
        for (volatile int d = 0; d < 100; d++) {}
    }
    DL_UART_disableLoopbackMode(UART_Tly_INST);
    bsp_uart_printf("[GYRO] Loopback %s\r\n", ok ? "PASS" : "FAIL");

    /* 屏蔽错误中断 */
    DL_UART_disableInterrupt(UART_Tly_INST,
        DL_UART_INTERRUPT_FRAMING_ERROR |
        DL_UART_INTERRUPT_PARITY_ERROR  |
        DL_UART_INTERRUPT_BREAK_ERROR   |
        DL_UART_INTERRUPT_OVERRUN_ERROR);

    /* 清 RX FIFO */
    while (!DL_UART_isRXFIFOEmpty(UART_Tly_INST))
        DL_UART_receiveData(UART_Tly_INST);

    NVIC_ClearPendingIRQ(UART_Tly_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Tly_INST_INT_IRQN);
}

void BspGyro::poll()
{
    uint8_t byte;
    while (pop_(&byte))
        parse_(byte);
}

float BspGyro::getWz()  { return wz_; }
float BspGyro::getYaw() { return yaw_; }

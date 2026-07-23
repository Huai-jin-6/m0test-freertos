/**
 * @file bsp_imu601.cpp
 * @brief 汇电籽-601 — UART2 ISR + 环形缓冲 + 12 字节帧协议解析
 */

#include "bsp_imu601.h"
#include "bsp_uart.h"
#include "ti_msp_dl_config.h"

/* ---- 全局实例 + ISR ---- */
BspImu601 bsp_imu601;

extern "C" void UART2_IRQHandler(void) { bsp_imu601.isrHandler(); }

/* ---- 环形缓冲 ---- */
void BspImu601::push_(uint8_t b)
{
    uint8_t n = (rxHead_ + 1) & 0x7F;
    if (n == rxTail_) return;
    rxBuf_[rxHead_] = b;
    rxHead_ = n;
}

int BspImu601::pop_(uint8_t *b)
{
    if (rxHead_ == rxTail_) return 0;
    *b = rxBuf_[rxTail_];
    rxTail_ = (rxTail_ + 1) & 0x7F;
    return 1;
}

/* ---- TX ---- */
void BspImu601::send_(const uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        DL_UART_transmitData(UART_IMU601_INST, data[i]);
        while (DL_UART_isBusy(UART_IMU601_INST)) {}
    }
}

/* ---- ISR ---- */
void BspImu601::isrHandler()
{
    switch (DL_UART_getPendingInterrupt(UART_IMU601_INST))
    {
        case DL_UART_IIDX_RX:
        {
            int limit = 32;
            while (limit-- > 0 && !DL_UART_isRXFIFOEmpty(UART_IMU601_INST))
            {
                uint8_t b = (uint8_t)DL_UART_receiveData(UART_IMU601_INST);
                rxTotal_++;
                push_(b);
            }
            break;
        }
        default: break;
    }
}

/* ---- 12 字节帧协议解析 ---- */
void BspImu601::parse_(uint8_t byte)
{
    static uint8_t buf[12];
    static uint8_t cnt = 0;
    static uint8_t last = 0;

    buf[cnt++] = byte;

    /* 等帧头 AA 55 */
    if (cnt == 2)
    {
        if (last != 0xAA || byte != 0x55) { cnt = 0; last = byte; return; }
    }
    last = byte;

    if (cnt < 12) return;

    /* 校验: sum(buf[2..10]) & 0xFF == buf[11] */
    uint8_t sum = 0;
    for (int i = 2; i <= 10; i++) sum += buf[i];
    if (sum != buf[11]) { cnt = 0; return; }

    if (buf[2] == 0x60 && buf[3] == 0x01 && buf[4] == 0x06)
    {
        yaw_   = (float)((uint16_t)(buf[5] | (buf[6] << 8))) / 100.0f;
        pitch_ = (float)((int16_t)(buf[7] | (buf[8] << 8))) / 100.0f;
        roll_  = (float)((int16_t)(buf[9] | (buf[10] << 8))) / 100.0f;
    }
    cnt = 0;
}

/* ---- 公开方法 ---- */

void BspImu601::reset()
{
    uint8_t cmd[] = {0xAA, 0x55, 0x60, 0x12, 0x00, 0x72};
    send_(cmd, sizeof(cmd));
    bsp_uart_printf("[IMU601] Reset\r\n");
}

void BspImu601::init()
{
    reset();

    DL_UART_disableInterrupt(UART_IMU601_INST,
        DL_UART_INTERRUPT_FRAMING_ERROR |
        DL_UART_INTERRUPT_PARITY_ERROR  |
        DL_UART_INTERRUPT_BREAK_ERROR   |
        DL_UART_INTERRUPT_OVERRUN_ERROR);

    while (!DL_UART_isRXFIFOEmpty(UART_IMU601_INST))
        DL_UART_receiveData(UART_IMU601_INST);

    NVIC_ClearPendingIRQ(UART_IMU601_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_IMU601_INST_INT_IRQN);
}

void BspImu601::poll()
{
    uint8_t byte;

    /* ring buffer (ISR 写入) */
    while (pop_(&byte))
        parse_(byte);

    /* FIFO 直接轮询兜底 */
    int limit = 32;
    while (limit-- > 0 && !DL_UART_isRXFIFOEmpty(UART_IMU601_INST))
    {
        byte = (uint8_t)DL_UART_receiveData(UART_IMU601_INST);
        rxTotal_++;
        parse_(byte);
    }
}

float BspImu601::getYaw()   { return yaw_; }
float BspImu601::getPitch() { return pitch_; }
float BspImu601::getRoll()  { return roll_; }

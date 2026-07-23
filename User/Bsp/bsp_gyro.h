/**
 * @file bsp_gyro.h
 * @brief WT901/JY901 陀螺仪类 — UART 通信 + 协议解析
 *
 * 协议: 0x5A 帧头, 5 字节, 0xAA=wz, 0xBB=yaw
 */

#ifndef __BSP_GYRO_HPP__
#define __BSP_GYRO_HPP__

#include <stdint.h>

class BspGyro
{
public:
    void init();
    void poll();
    void isrHandler();          ///< UART ISR 回调

    float    getWz();
    float    getYaw();
    uint32_t rxTotal() const { return rxTotal_; }

private:
    static constexpr uint8_t RX_BUF_SIZE = 128;
    uint8_t  rxBuf_[RX_BUF_SIZE];
    uint8_t  rxHead_ = 0, rxTail_ = 0;
    uint32_t rxTotal_ = 0;
    float    wz_  = 0, yaw_ = 0;

    void push_(uint8_t b);
    int  pop_(uint8_t *b);
    void parse_(uint8_t byte);
};

extern BspGyro bsp_gyro;

#endif

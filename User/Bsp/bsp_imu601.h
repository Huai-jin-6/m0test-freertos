/**
 * @file bsp_imu601.h
 * @brief 汇电籽-601 (ICM42688) 类 — UART 通信 + 协议解析
 *
 * 协议: 12 字节帧, AA 55 60 01 06 [6B Payload] [Checksum]
 */

#ifndef __BSP_IMU601_HPP__
#define __BSP_IMU601_HPP__

#include <stdint.h>

class BspImu601
{
public:
    void init();
    void reset();
    void poll();
    void isrHandler();

    float    getYaw();
    float    getPitch();
    float    getRoll();
    uint32_t rxTotal() const { return rxTotal_; }

private:
    static constexpr uint8_t RX_BUF_SIZE = 128;
    uint8_t  rxBuf_[RX_BUF_SIZE];
    uint8_t  rxHead_ = 0, rxTail_ = 0;
    uint32_t rxTotal_ = 0;
    float    yaw_ = 0, pitch_ = 0, roll_ = 0;

    void push_(uint8_t b);
    int  pop_(uint8_t *b);
    void send_(const uint8_t *data, uint8_t len);
    void parse_(uint8_t byte);
};

extern BspImu601 bsp_imu601;

#endif

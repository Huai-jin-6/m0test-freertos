/**
 * @file bsp_i2c.h
 * @brief BSP I2C 抽象 — 主机发送 (MSPM0 I2C0, PA0=SDA, PA1=SCL)
 *
 * 换 MCU 时只需重写 bsp_i2c.cpp，接口保持不变
 */

#ifndef __BSP_I2C_HPP__
#define __BSP_I2C_HPP__

#include <stdint.h>

class BspI2c
{
public:
    void init();
    void send(uint8_t addr, const uint8_t *buf, uint8_t len);  ///< 主机发送，阻塞
};

#endif

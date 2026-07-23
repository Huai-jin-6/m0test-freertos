/**
 * @file bsp_vl53l0x_i2c.h
 * @brief VL53L0X GPIO 模拟 I2C — Bsp 层 (PA15=SCL, PA16=SDA, PA31=XSHUT)
 *
 * 换 MCU 只需重写 bsp_vl53l0x_i2c.cpp，实现全部方法
 */

#ifndef __BSP_VL53L0X_I2C_HPP__
#define __BSP_VL53L0X_I2C_HPP__

#include <stdint.h>
#include <stdbool.h>

class BspVl53l0xI2c
{
public:
    void init();       ///< 初始化 GPIO + XSHUT
    void delayUs(uint32_t us);

    bool writeByte(uint8_t addr, uint8_t reg, uint8_t data);
    bool readByte(uint8_t addr, uint8_t reg, uint8_t *data);
    bool writeMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
    bool readMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);

private:
    bool inited_ = false;

    void sdaOut_();
    void sdaIn_();
    void start_();
    void stop_();
    bool waitAck_();
    void sendByte_(uint8_t d);
    uint8_t readByte_(bool ack);
};

extern BspVl53l0xI2c vl53l0x_i2c;

#endif

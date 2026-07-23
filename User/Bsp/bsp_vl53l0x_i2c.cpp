/**
 * @file bsp_vl53l0x_i2c.cpp
 * @brief VL53L0X GPIO 模拟 I2C — MSPM0 实现 (SysConfig 引脚 VL 组)
 */

#include "bsp_vl53l0x_i2c.h"
#include "ti_msp_dl_config.h"

BspVl53l0xI2c vl53l0x_i2c;

/* ---- 引脚宏 ---- */
#define SCL_PORT  VL_PORT
#define SDA_PORT  VL_PORT
#define SCL_PIN   VL_VL_SCL_PIN
#define SDA_PIN   VL_VL_SDA_PIN
#define SCL_IOMUX VL_VL_SCL_IOMUX
#define SDA_IOMUX VL_VL_SDA_IOMUX

#define DELAY  delay_cycles(80)

#define SDA_H()  DL_GPIO_setPins(SDA_PORT, SDA_PIN)
#define SDA_L()  DL_GPIO_clearPins(SDA_PORT, SDA_PIN)
#define SCL_H()  DL_GPIO_setPins(SCL_PORT, SCL_PIN)
#define SCL_L()  DL_GPIO_clearPins(SCL_PORT, SCL_PIN)
#define SDA_RD() (DL_GPIO_readPins(SDA_PORT, SDA_PIN) != 0)

/* ---- 方向切换 ---- */
void BspVl53l0xI2c::sdaOut_()
{
    DL_GPIO_initDigitalOutput(SDA_IOMUX);
    DL_GPIO_enableOutput(SDA_PORT, SDA_PIN);
}
void BspVl53l0xI2c::sdaIn_()
{
    DL_GPIO_disableOutput(SDA_PORT, SDA_PIN);
    DL_GPIO_initDigitalInput(SDA_IOMUX);
}

/* ---- I2C 时序 ---- */
void BspVl53l0xI2c::start_()
{
    sdaOut_(); SDA_H(); DELAY; SCL_H(); DELAY; SDA_L(); DELAY; SCL_L(); DELAY;
}
void BspVl53l0xI2c::stop_()
{
    sdaOut_(); SCL_L(); SDA_L(); DELAY; SCL_H(); DELAY; SDA_H(); DELAY;
}
bool BspVl53l0xI2c::waitAck_()
{
    sdaIn_();
    SCL_L(); DELAY; SCL_H();
    uint32_t to = 5000;
    while (SDA_RD() && --to) { delay_cycles(2); }
    SCL_L(); sdaOut_(); DELAY;
    return to == 0;
}
void BspVl53l0xI2c::sendByte_(uint8_t d)
{
    sdaOut_();
    for (int i = 0; i < 8; i++) {
        SCL_L();
        if (d & 0x80) SDA_H(); else SDA_L();
        DELAY; SCL_H(); DELAY;
        d <<= 1;
    }
    SCL_L();
}
uint8_t BspVl53l0xI2c::readByte_(bool ack)
{
    uint8_t d = 0;
    sdaIn_();
    for (int i = 0; i < 8; i++) {
        SCL_L(); DELAY; SCL_H(); DELAY;
        d <<= 1;
        if (SDA_RD()) d |= 1;
    }
    SCL_L();
    sdaOut_();
    if (ack) SDA_L(); else SDA_H();
    DELAY; SCL_H(); DELAY; SCL_L(); DELAY;
    SDA_H();
    return d;
}

/* ================================================================
 *  公开 API
 * ================================================================ */

void BspVl53l0xI2c::init()
{
    if (inited_) return;

    /* XSHUT 拉高 */
    DL_GPIO_initDigitalOutput(VL_VL_XSHUT_IOMUX);
    DL_GPIO_enableOutput(VL_PORT, VL_VL_XSHUT_PIN);
    DL_GPIO_setPins(VL_PORT, VL_VL_XSHUT_PIN);
    delayUs(500);

    /* SCL / SDA */
    DL_GPIO_initDigitalOutput(SCL_IOMUX);
    DL_GPIO_enableOutput(SCL_PORT, SCL_PIN);
    DL_GPIO_initDigitalOutput(SDA_IOMUX);
    DL_GPIO_enableOutput(SDA_PORT, SDA_PIN);
    DL_GPIO_setPins(SCL_PORT, SCL_PIN);
    DL_GPIO_setPins(SDA_PORT, SDA_PIN);
    inited_ = true;
}

void BspVl53l0xI2c::delayUs(uint32_t us)
{
    while (us--) delay_cycles(80);
}

bool BspVl53l0xI2c::writeByte(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t buf[1] = { data };
    return writeMulti(addr, reg, buf, 1);
}

bool BspVl53l0xI2c::readByte(uint8_t addr, uint8_t reg, uint8_t *data)
{
    return readMulti(addr, reg, data, 1);
}

bool BspVl53l0xI2c::writeMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    start_();
    sendByte_(addr);
    if (waitAck_()) { stop_(); return false; }
    sendByte_(reg);
    if (waitAck_()) { stop_(); return false; }
    while (len--) {
        sendByte_(*buf++);
        if (waitAck_()) { stop_(); return false; }
    }
    stop_();
    return true;
}

bool BspVl53l0xI2c::readMulti(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    start_();
    sendByte_(addr);
    if (waitAck_()) { stop_(); return false; }
    sendByte_(reg);
    if (waitAck_()) { stop_(); return false; }

    start_();
    sendByte_(addr | 0x01);
    if (waitAck_()) { stop_(); return false; }

    while (len--)
        *buf++ = readByte_(len > 0);

    stop_();
    return true;
}

/**
 * @file bsp_i2c.cpp
 * @brief BSP I2C — MSPM0 I2C0 实现 (500kHz Fast)
 */

#include "bsp_i2c.h"
#include "ti_msp_dl_config.h"

void BspI2c::init()
{
    /* I2C0 已在 SYSCFG_DL_init 中配好 */
}

void BspI2c::send(uint8_t addr, const uint8_t *buf, uint8_t len)
{
    /* 等上次传输完成 */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE)) { }

    DL_I2C_fillControllerTXFIFO(I2C_0_INST, buf, len);

    DL_I2C_startControllerTransfer(I2C_0_INST, addr,
                                    DL_I2C_CONTROLLER_DIRECTION_TX, len);

    /* 等本次传输完成 */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE)) { }
}

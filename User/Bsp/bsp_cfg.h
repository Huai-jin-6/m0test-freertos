/**
 * @file bsp_cfg.h
 * @brief BSP 全局配置 — 所有外设实例声明 + 一键初始化
 */

#ifndef __BSP_CFG_HPP__
#define __BSP_CFG_HPP__

#include "bsp_spi.h"
#include "bsp_i2c.h"
#include "bsp_pwm.h"
#include "bsp_can.h"
#include "bsp_uart.h"

struct BspCfg
{
    BspSpi  spi;
    BspI2c  i2c;
    BspPwm  pwm;
    BspCan  can;
    BspUart uart;

    void init();
};

extern BspCfg bsp;

#endif

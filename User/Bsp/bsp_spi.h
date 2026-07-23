/**
 * @file bsp_spi.h
 * @brief BSP SPI 抽象 (MSPM0 SPI1, Mode 3, 8MHz)
 *
 * 换 MCU 时只需重写 bsp_spi.cpp，接口保持不变
 */

#ifndef __BSP_SPI_HPP__
#define __BSP_SPI_HPP__

#include <stdint.h>

class BspSpi
{
public:
    void     init();
    uint8_t  transfer(uint8_t tx);  ///< 全双工单字节，超时返回 0
    void     csHigh();              ///< 软件片选拉高 (PB6)
    void     csLow();               ///< 软件片选拉低
};

#endif

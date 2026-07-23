/**
 * @file bsp_spi.cpp
 * @brief BSP SPI — MSPM0 SPI1 实现 (Mode 3, 8MHz)
 *
 * 引脚：PB7=POCI, PB8=PICO, PB9=SCLK, PB6=CS（软件片选）
 */

#include "bsp_spi.h"
#include "ti_msp_dl_config.h"

static const uint32_t SPI_TIMEOUT = 65535;

void BspSpi::init()
{
    DL_GPIO_setPins(SPI_CS_PORT, SPI_CS_SPI_CS0_PIN);  /* CS 初始高 */
}

uint8_t BspSpi::transfer(uint8_t tx)
{
    uint32_t to;

    to = SPI_TIMEOUT;
    while (DL_SPI_isTXFIFOFull(SPI_1_INST) && --to) { }
    if (to == 0) return 0;

    DL_SPI_transmitData8(SPI_1_INST, tx);

    to = SPI_TIMEOUT;
    while (DL_SPI_isRXFIFOEmpty(SPI_1_INST) && --to) { }
    if (to == 0) return 0;

    return DL_SPI_receiveData8(SPI_1_INST);
}

void BspSpi::csHigh() { DL_GPIO_setPins(SPI_CS_PORT, SPI_CS_SPI_CS0_PIN); }
void BspSpi::csLow()  { DL_GPIO_clearPins(SPI_CS_PORT, SPI_CS_SPI_CS0_PIN); }

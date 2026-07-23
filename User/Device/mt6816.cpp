/**
 * @file mt6816.cpp
 * @brief MT6816 磁编码器驱动 — SPI 角度读取（器件层）
 *
 * 通信协议（SPI Mode 3, CPOL=1, CPHA=1）：
 *   1. CS 拉低 → 发 0x83 + dummy → 收到角度高 8 位 [13:6]
 *   2. CS 拉高 → CS 拉低 → 发 0x84 + dummy → 收到角度低 6 位 [5:0]
 *   3. 角度拼接：raw = ((high << 8) | low) >> 2
 *   4. 零值毛刺过滤：跳 0 且 delta > 90° → 丢弃
 */

#include "mt6816.h"
#include "bsp_spi.h"

bool Mt6816::init(const Config &cfg)
{
    if (!cfg.spi) return false;
    _spi = cfg.spi;
    _spi->csHigh();
    return true;
}

void Mt6816::read(MT6816_Data *data)
{
    uint8_t data_high, data_low;

    /* ---- 读角度高 8 位 ---- */
    _spi->csLow();
    _spi->transfer(REG_HIGH);
    data_high = _spi->transfer(0x00);
    _spi->csHigh();

    /* ---- 读角度低 6 位 ---- */
    _spi->csLow();
    _spi->transfer(REG_LOW);
    data_low = _spi->transfer(0x00);
    _spi->csHigh();

    /* SPI 通信异常：两次读回均为超时返回 0 或全 0xFF，丢弃本次 */
    if ((data_high == 0 && data_low == 0) || (data_high == 0xFF && data_low == 0xFF))
        return;

    /* ---- 保存原始数据 ---- */
    data->raw_h = data_high;
    data->raw_l = data_low;
    data->mag   = (data_low & 0x02) >> 1;

    /* ---- 磁铁正常才更新角度 ---- */
    if (data->mag == 0)
    {
        int new_raw = ((data_high << 8) | data_low) >> 2;

        /* 毛刺检测：跳到 0 且 delta > 90°(4096 raw)，不更新 */
        int delta = new_raw - data->raw_angle;
        if (delta < 0) delta = -delta;
        if (delta > 8192) delta = 16384 - delta;

        if (!(new_raw == 0 && delta > 4096))
        {
            data->raw_angle = new_raw;
            data->angle     = data->raw_angle * 360.0f / 16384.0f;
        }
    }
}

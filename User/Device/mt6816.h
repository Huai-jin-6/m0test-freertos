/**
 * @file mt6816.h
 * @brief MT6816 14-bit 磁编码器 — 器件层类封装
 *
 * 依赖：BspSpi
 * 与 MCU 无关，换平台只需 Bsp 层实现同名函数
 */

#ifndef __MT6816_HPP__
#define __MT6816_HPP__

#include <stdint.h>

/** @brief 编码器数据结构体 */
typedef struct
{
    uint8_t mag;        ///< 磁铁检测标志：0=正常, 1=异常
    uint8_t raw_h;      ///< SPI 读回的原始高字节
    uint8_t raw_l;      ///< SPI 读回的原始低字节
    int     raw_angle;  ///< 拼接后的 14 位原始角度值（0~16383）
    float   angle;      ///< 角度制（0°~360°）
} MT6816_Data;

#ifdef __cplusplus

class BspSpi;

class Mt6816
{
public:
    struct Config
    {
        BspSpi *spi = nullptr;
    };

    bool init(const Config &cfg);
    void read(MT6816_Data *data);

private:
    BspSpi *_spi = nullptr;

    static constexpr uint8_t REG_HIGH = 0x83;  ///< 读 0x03：角度高 8 位
    static constexpr uint8_t REG_LOW  = 0x84;  ///< 读 0x04：角度低 6 位 + 状态
};

#endif /* __cplusplus */

#endif

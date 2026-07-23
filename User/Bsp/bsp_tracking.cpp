/**
 * @file bsp_tracking.cpp
 * @brief 八通道灰度循迹 — GPIO 读取 74HC165D
 */

#include "bsp_tracking.h"
#include "ti_msp_dl_config.h"

#define T_DELAY  delay_cycles(80)   /* ~1μs @ 80MHz */

void Tracking::init()
{
    DL_GPIO_setPins(Gray_PL_PORT, Gray_PL_PIN);
    DL_GPIO_clearPins(Gray_SCK_PORT, Gray_SCK_PIN);
}

uint8_t Tracking::readRaw_()
{
    uint8_t data = 0;

    /* PL 脉冲：锁存并行输入 */
    DL_GPIO_clearPins(Gray_PL_PORT, Gray_PL_PIN);
    T_DELAY;
    DL_GPIO_setPins(Gray_PL_PORT, Gray_PL_PIN);
    T_DELAY;

    /* 读第一 bit (Q7) */
    if (DL_GPIO_readPins(Gray_SDA_PORT, Gray_SDA_PIN) != 0)
        data |= (1U << 7);

    DL_GPIO_clearPins(Gray_SCK_PORT, Gray_SCK_PIN);
    T_DELAY;

    /* 余下 7 bit */
    for (uint8_t i = 0; i < 7; i++)
    {
        DL_GPIO_setPins(Gray_SCK_PORT, Gray_SCK_PIN);
        T_DELAY;

        if (DL_GPIO_readPins(Gray_SDA_PORT, Gray_SDA_PIN) != 0)
            data |= (1U << (6 - i));

        DL_GPIO_clearPins(Gray_SCK_PORT, Gray_SCK_PIN);
        T_DELAY;
    }

    return data;
}

void Tracking::poll()
{
    data_ = ~readRaw_();   /* 取反：1=黑线 */
}

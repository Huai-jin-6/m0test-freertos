/**
 * @file bsp_tracking.h
 * @brief 八通道灰度循迹类 — GPIO 读取 74HC165D
 *
 * 引脚: PB3=PL, PB2=SCK, PA7=SDA
 */

#ifndef __BSP_TRACKING_HPP__
#define __BSP_TRACKING_HPP__

#include <stdint.h>

class Tracking
{
public:
    void    init();
    void    poll();          ///< 每 1ms 调用，读并缓存
    uint8_t get() const { return data_; }  ///< 取反后: bit=1 表示黑线

private:
    uint8_t data_ = 0;
    uint8_t readRaw_();     ///< 读 74HC165，阻塞 ~10μs
};

#endif

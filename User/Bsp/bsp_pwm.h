/**
 * @file bsp_pwm.h
 * @brief BSP PWM 抽象 — 步进电机 STEP 脉冲 (TIMG7, PA26)
 *
 * 换 MCU 时只需重写 bsp_pwm.cpp，接口保持不变
 */

#ifndef __BSP_PWM_HPP__
#define __BSP_PWM_HPP__

#include <stdint.h>

class BspPwm
{
public:
    void init();
    void set_freq(uint32_t hz);  ///< 设频率 Hz, 0=停, 50% 占空比
};

#endif

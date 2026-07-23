/**
 * @file bsp_pwm.cpp
 * @brief BSP PWM — MSPM0 TIMG7 实现 (1MHz, 50% 占空比)
 */

#include "bsp_pwm.h"
#include "ti_msp_dl_config.h"

static const uint32_t TIMER_CLK  = 1000000U;   /* 1MHz = 80MHz/80 */
static const uint32_t MIN_PERIOD = 10U;

void BspPwm::init()
{
    DL_TimerG_stopCounter(PWM_Step_INST);
}

void BspPwm::set_freq(uint32_t hz)
{
    if (hz == 0)
    {
        __disable_irq();
        DL_TimerG_stopCounter(PWM_Step_INST);
        __enable_irq();
        return;
    }

    uint32_t period = TIMER_CLK / hz;
    if (period > 65535U) period = 65535U;
    if (period < MIN_PERIOD) period = MIN_PERIOD;

    __disable_irq();
    DL_TimerG_setLoadValue(PWM_Step_INST, period);
    DL_TimerG_setCaptureCompareValue(PWM_Step_INST, period / 2,
                                      DL_TIMER_CC_0_INDEX);
    DL_TimerG_startCounter(PWM_Step_INST);
    __enable_irq();
}

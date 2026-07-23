/**
 * @file stepper_motor.cpp
 * @brief 步进电机 STEP+DIR — 速度公式: freq = RPM × 6400 / 60
 */

#include "stepper_motor.h"
#include "bsp_pwm.h"
#include "ti_msp_dl_config.h"

#define DIR_PORT  STP_pins_PORT
#define DIR_PIN   STP_pins_Dir_PIN

bool StepperMotor::init(const Config &cfg)
{
    if (!cfg.pwm) return false;
    _pwm = cfg.pwm;
    DL_GPIO_setPins(DIR_PORT, DIR_PIN);
    _pwm->init();
    return true;
}

void StepperMotor::set_speed(float rpm)
{
    /* 方向 */
    if (rpm >= 0.0f)
        DL_GPIO_setPins(DIR_PORT, DIR_PIN);
    else {
        DL_GPIO_clearPins(DIR_PORT, DIR_PIN);
        rpm = -rpm;
    }

    /* 停止 (< 0.5 RPM) */
    if (rpm < 0.5f) {
        _pwm->set_freq(0);
        return;
    }

    /* RPM → 脉冲频率(Hz) */
    uint32_t freq = (uint32_t)(rpm * (float)STEPS_PER_REV / 60.0f);
    _pwm->set_freq(freq);
}

void StepperMotor::enable(bool on)
{
#ifdef EN_PORT
    if (on)
        DL_GPIO_clearPins(EN_PORT, EN_En_pin_PIN);
    else
        DL_GPIO_setPins(EN_PORT, EN_En_pin_PIN);
#else
    (void)on;
#endif
}

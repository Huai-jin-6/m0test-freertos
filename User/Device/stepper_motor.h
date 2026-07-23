/**
 * @file stepper_motor.h
 * @brief 步进电机 STEP+DIR 驱动（器件层）
 *
 * 依赖：BspPwm (STEP 脉冲)、bsp_gpio (DIR 方向)
 */

#ifndef __STEPPER_MOTOR_HPP__
#define __STEPPER_MOTOR_HPP__

#include <stdint.h>
#include <stdbool.h>

#define STEPS_PER_REV   6400    ///< 200 × 32 细分

#ifdef __cplusplus

class BspPwm;

class StepperMotor
{
public:
    struct Config
    {
        BspPwm *pwm = nullptr;
    };

    bool init(const Config &cfg);
    void set_speed(float rpm);   ///< >0=正转, <0=反转, |rpm|<0.5=停
    void enable(bool on);        ///< 使能/失能驱动器

private:
    BspPwm *_pwm = nullptr;
};

#endif /* __cplusplus */

#endif

/**
 * @file ins.cpp
 * @brief 2D 惯性导航 — 单陀螺仪航向 + 电机里程计航位推算
 */

#include "ins.h"
#include "bsp_gyro.h"
#include "bsp_imu601.h"
#include "motor_control.hpp"
#include <math.h>

extern MotorControl g_motor;
extern BspGyro     bsp_gyro;
extern BspImu601   bsp_imu601;

void Ins::init()
{
    gyro_     = NONE;
    heading_  = 0;
    x_ = y_   = 0;
    speed_    = 0;
    mmPerRev_ = 0;
}

void Ins::useWt901()   { gyro_ = WT901; }
void Ins::useImu601()  { gyro_ = IMU601; }
void Ins::setWheelMmPerRev(float mm) { mmPerRev_ = mm; }
void Ins::reset()      { x_ = y_ = 0; heading_ = 0; }

float Ins::heading()  const { return heading_; }
float Ins::x()        const { return x_; }
float Ins::y()        const { return y_; }
float Ins::speedMmS() const { return speed_; }

void Ins::tick(float dt_s)
{
    float yaw = 0;

    /* ---- 1. 航向 ---- */
    if (gyro_ == WT901)
    {
        float wz = bsp_gyro.getWz();
        yaw      = bsp_gyro.getYaw();
        heading_ += wz * dt_s;
        heading_ += K_YAW * (yaw - heading_);

        if      (heading_ >  180.0f) heading_ -= 360.0f;
        else if (heading_ < -180.0f) heading_ += 360.0f;
    }
    else if (gyro_ == IMU601)
    {
        heading_ = bsp_imu601.getYaw();
    }

    /* ---- 2. 航位推算 ---- */
    if (mmPerRev_ <= 0) return;

    float rpm = g_motor.speed();
    speed_    = rpm / 60.0f * mmPerRev_;

    float rad = heading_ * 3.14159265f / 180.0f;

    x_ += speed_ * cosf(rad) * dt_s;
    y_ += speed_ * sinf(rad) * dt_s;
}

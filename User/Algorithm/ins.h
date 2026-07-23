/**
 * @file ins.h
 * @brief 2D 惯性导航类 — 单陀螺仪航向 + 电机里程计航位推算
 */

#ifndef __INS_HPP__
#define __INS_HPP__

#include <stdint.h>

class Ins
{
public:
    void init();
    void useWt901();       ///< WT901: wz 积分 + yaw 互补
    void useImu601();      ///< IMU601: 纯 yaw
    void tick(float dt_s); ///< 每 1ms

    void  setWheelMmPerRev(float mm);  ///< 0=不积位置
    void  reset();                     ///< 位置归零

    float heading() const;   ///< 航向 °
    float x() const;         ///< X mm
    float y() const;         ///< Y mm
    float speedMmS() const;  ///< 线速度 mm/s

private:
    enum GyroType { NONE, WT901, IMU601 };
    GyroType gyro_    = NONE;
    float    heading_ = 0;
    float    x_       = 0;
    float    y_       = 0;
    float    speed_   = 0;
    float    mmPerRev_ = 0;

    static constexpr float K_YAW = 0.01f;
};

#endif

/**
 * @file vl53l0x_sensor.h
 * @brief VL53L0X 激光测距类 — 非阻塞状态机
 *
 * 接线: PA15=SCL, PA16=SDA, PA31=XSHUT
 */

#ifndef __VL53L0X_SENSOR_HPP__
#define __VL53L0X_SENSOR_HPP__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus

class Vl53l0x
{
public:
    bool     init();              ///< 初始化（阻塞 ~100ms）
    void     tick();              ///< 每 1ms: 状态机推进
    uint16_t get() const { return distance_; }  ///< 最新距离 mm, 0xFFFF=无数据

private:
    enum State { IDLE, START, WAITING, READ_RESULT };
    State    state_    = IDLE;
    uint16_t distance_ = 0xFFFF;

    /* 保留旧 C 接口兼容 */
public:
    uint16_t range();  ///< 阻塞版本，调试用
};

#endif /* __cplusplus */

/* 弱函数：应用层覆盖以接入 printf */
void vl53l0x_log(const char *msg);

#endif

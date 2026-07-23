/**
 * @file vl53l0x_sensor.cpp
 * @brief VL53L0X 测距类 — 非阻塞状态机
 */

#include "vl53l0x_sensor.h"
#include "vl53l0x/vl53l0x_api.h"
#include "vl53l0x/vl53l0x_api_calibration.h"
#include "vl53l0x/vl53l0x_api_ranging.h"

__attribute__((weak))
void vl53l0x_log(const char *msg) { (void)msg; }

static VL53L0X_Dev_t g_dev;

bool Vl53l0x::init()
{
    /* GPIO I2C 已在 vl53l0x_i2c.init() 中初始化（bsp.init 调用） */
    VL53L0X_Error Status;

    g_dev.I2cDevAddr    = 0x52;
    g_dev.comms_type    = 1;
    g_dev.comms_speed_khz = 400;

    uint16_t id;
    Status = VL53L0X_RdWord(&g_dev, 0xC0, &id);
    if (Status || id != 0xEEAA) { vl53l0x_log("VL: no dev"); return false; }

    Status = VL53L0X_DataInit(&g_dev);
    if (Status) { vl53l0x_log("VL: datainit err"); return false; }

    uint8_t VhvSettings, PhaseCal;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;

    Status = VL53L0X_StaticInit(&g_dev);
    if (Status) { vl53l0x_log("VL: stat err"); return false; }

    Status = VL53L0X_PerformRefCalibration(&g_dev, &VhvSettings, &PhaseCal);
    if (Status) { vl53l0x_log("VL: refcal err"); return false; }

    Status = VL53L0X_PerformRefSpadManagement(&g_dev, &refSpadCount, &isApertureSpads);
    if (Status) { vl53l0x_log("VL: spad err"); return false; }

    Status = VL53L0X_SetDeviceMode(&g_dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);
    if (Status) { vl53l0x_log("VL: mode err"); return false; }

    Status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_dev, 33000);
    if (Status) { vl53l0x_log("VL: budget err"); return false; }

    vl53l0x_log("VL53L0X OK");
    state_ = IDLE;
    return true;
}

void Vl53l0x::tick()
{
    uint8_t ready;

    switch (state_)
    {
    case IDLE:
        {
            static uint16_t cooldown = 0;
            if (++cooldown < 50) return;
            cooldown = 0;
        }
        VL53L0X_StartMeasurement(&g_dev);
        state_ = START;
        break;

    case START:
        state_ = WAITING;
        break;

    case WAITING:
        VL53L0X_GetMeasurementDataReady(&g_dev, &ready);
        if (ready) state_ = READ_RESULT;
        break;

    case READ_RESULT:
        {
            VL53L0X_RangingMeasurementData_t data;
            VL53L0X_GetRangingMeasurementData(&g_dev, &data);
            VL53L0X_ClearInterruptMask(&g_dev, 0);
            distance_ = (data.RangeStatus == 0) ? data.RangeMilliMeter : 0xFFFF;
        }
        state_ = IDLE;
        break;
    }
}

uint16_t Vl53l0x::range()
{
    VL53L0X_RangingMeasurementData_t data;
    VL53L0X_Error Status = VL53L0X_PerformSingleRangingMeasurement(&g_dev, &data);
    return (Status == VL53L0X_ERROR_NONE) ? data.RangeMilliMeter : 0xFFFF;
}

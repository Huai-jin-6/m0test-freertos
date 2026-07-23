/**
 * @file vl53l0x_platform.cpp
 * @brief ST VL53L0X API → BspVl53l0xI2c 平台适配
 */

#include "vl53l0x_platform.h"
#include "bsp_vl53l0x_i2c.h"

extern BspVl53l0xI2c vl53l0x_i2c;

VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    if (count >= VL53L0X_MAX_I2C_XFER_SIZE) return VL53L0X_ERROR_INVALID_PARAMS;
    if (!vl53l0x_i2c.writeMulti(Dev->I2cDevAddr, index, pdata, (uint16_t)count))
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count)
{
    if (count >= VL53L0X_MAX_I2C_XFER_SIZE) return VL53L0X_ERROR_INVALID_PARAMS;
    if (!vl53l0x_i2c.readMulti(Dev->I2cDevAddr, index, pdata, (uint16_t)count))
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data) {
    return vl53l0x_i2c.writeByte(Dev->I2cDevAddr, index, data) ? VL53L0X_ERROR_NONE : VL53L0X_ERROR_CONTROL_INTERFACE;
}
VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data) {
    uint8_t b[2]={(uint8_t)(data>>8),(uint8_t)data};
    if(index&1){vl53l0x_i2c.writeByte(Dev->I2cDevAddr,index,b[0]);vl53l0x_i2c.writeByte(Dev->I2cDevAddr,index+1,b[1]);}
    else vl53l0x_i2c.writeMulti(Dev->I2cDevAddr,index,b,2);
    return VL53L0X_ERROR_NONE;
}
VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data) {
    uint8_t b[4]={(uint8_t)(data>>24),(uint8_t)(data>>16),(uint8_t)(data>>8),(uint8_t)data};
    vl53l0x_i2c.writeMulti(Dev->I2cDevAddr,index,b,4);
    return VL53L0X_ERROR_NONE;
}
VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data) {
    return vl53l0x_i2c.readByte(Dev->I2cDevAddr,index,data) ? VL53L0X_ERROR_NONE : VL53L0X_ERROR_CONTROL_INTERFACE;
}
VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data) {
    uint8_t b[2];
    if(!vl53l0x_i2c.readMulti(Dev->I2cDevAddr,index,b,2)) return VL53L0X_ERROR_CONTROL_INTERFACE;
    *data=((uint16_t)b[0]<<8)|b[1];
    return VL53L0X_ERROR_NONE;
}
VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data) {
    uint8_t b[4];
    if(!vl53l0x_i2c.readMulti(Dev->I2cDevAddr,index,b,4)) return VL53L0X_ERROR_CONTROL_INTERFACE;
    *data=((uint32_t)b[0]<<24)|((uint32_t)b[1]<<16)|((uint32_t)b[2]<<8)|b[3];
    return VL53L0X_ERROR_NONE;
}
VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData) {
    uint8_t d; VL53L0X_Error e=VL53L0X_RdByte(Dev,index,&d);
    if(e) return e; d=(d&AndData)|OrData; return VL53L0X_WrByte(Dev,index,d);
}
VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev) { (void)Dev; vl53l0x_i2c.delayUs(100); return VL53L0X_ERROR_NONE; }

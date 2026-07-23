#ifndef __VL53L0X_TYPES_H__
#define __VL53L0X_TYPES_H__
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef NULL
#define NULL 0
#endif
typedef uint32_t FixPoint1616_t;
#define VL53L0X_API
/* 禁用 ST API 内部日志 (Keil/STM32 用) */
#define TRACE_MODULE_API 0
#define _LOG_FUNCTION_START(m, ...)  (void)0
#define _LOG_FUNCTION_END(m, s, ...) (void)0
#define LOG_FUNCTION_START(...)
#define LOG_FUNCTION_END(...)
#define LOG_FUNCTION_END_FMT(...)
#define VL53L0X_COPYSTRING(str, ...)  (void)0
#endif

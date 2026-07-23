/**
 * @file bsp_gpio.h
 * @brief BSP GPIO 工具 — LED 闪烁 + 微秒延时
 */

#ifndef __BSP_GPIO_HPP__
#define __BSP_GPIO_HPP__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void bsp_led_toggle(void);
void bsp_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @file bsp_gpio.cpp
 * @brief BSP GPIO — LED + 延时
 */

#include "bsp_gpio.h"
#include "ti_msp_dl_config.h"

void bsp_led_toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_LED22_PIN);
}

void bsp_delay_us(uint32_t us)
{
    for (volatile uint32_t i = 0; i < us * 60; i++)
        __asm("nop");
}

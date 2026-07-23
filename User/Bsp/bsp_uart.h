/**
 * @file bsp_uart.h
 * @brief BSP UART 调试输出 (MSPM0 UART0, PA10=TX, PA11=RX)
 *
 * 提供 C++ 类封装 + C 兼容的 bsp_uart_printf() 便捷函数
 */

#ifndef __BSP_UART_HPP__
#define __BSP_UART_HPP__

#include <stdint.h>

class BspUart
{
public:
    void init();
    int  printf(const char *fmt, ...);
};

/* 全局实例 */
extern BspUart bsp_uart;

/* C 兼容便捷函数 — 底层调用 bsp_uart.printf() */
#ifdef __cplusplus
extern "C" {
#endif
int bsp_uart_printf(const char *fmt, ...);
#ifdef __cplusplus
}
#endif

#endif

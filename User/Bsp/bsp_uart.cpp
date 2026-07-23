/**
 * @file bsp_uart.cpp
 * @brief BSP UART — MSPM0 UART0 printf + RX drain ISR
 */

#include "bsp_uart.h"
#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <stdarg.h>

/* ---- 全局实例 ---- */
BspUart bsp_uart;

/* ---- ISR: 吃掉 RX 字节防止 Default_Handler ---- */
extern "C" void UART0_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_DEBUG_INST))
    {
        case DL_UART_IIDX_RX:
        {
            int limit = 32;
            while (limit-- > 0 && !DL_UART_isRXFIFOEmpty(UART_DEBUG_INST))
                DL_UART_receiveData(UART_DEBUG_INST);
            break;
        }
        default:
            break;
    }
}

/* ---- BspUart 方法 ---- */

void BspUart::init()
{
    /* UART0 已在 SYSCFG_DL_init 中配好 */
}

int BspUart::printf(const char *fmt, ...)
{
    static char buf[128];
    va_list args;
    va_start(args, fmt);
    uint32_t len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (uint32_t i = 0; i < len; i++)
    {
        DL_UART_transmitData(UART_DEBUG_INST, buf[i]);
        while (!DL_UART_isTXFIFOEmpty(UART0))
            ;
    }
    return len;
}

/* ---- C 兼容便捷函数 ---- */

int bsp_uart_printf(const char *fmt, ...)
{
    static char buf[128];
    va_list args;
    va_start(args, fmt);
    uint32_t len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (uint32_t i = 0; i < len; i++)
    {
        DL_UART_transmitData(UART_DEBUG_INST, buf[i]);
        while (!DL_UART_isTXFIFOEmpty(UART0))
            ;
    }
    return len;
}

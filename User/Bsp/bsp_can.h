/**
 * @file bsp_can.h
 * @brief CAN 通信 BSP 层 — 非阻塞发送 + 环形接收缓冲 + ISR
 *
 * PA12=CAN_TX, PA13=CAN_RX, 250kbps
 */

#ifndef __BSP_CAN_HPP__
#define __BSP_CAN_HPP__

#include <stdint.h>
#include <stdbool.h>

class BspCan
{
public:
    void init();
    void enable_irq();
    void disable_irq();

    bool send(uint32_t id, const uint8_t *data, uint8_t len);
    bool recv_read(uint32_t *id, uint8_t *data, uint8_t *len);

    /* ISR 回调，由 MCAN0_INST_IRQHandler 调用 */
    void isr_handler();

private:
    static constexpr uint8_t RX_BUF_SIZE = 16;

    uint32_t _rx_id[RX_BUF_SIZE];
    uint8_t  _rx_data[RX_BUF_SIZE][8];
    uint8_t  _rx_len[RX_BUF_SIZE];
    uint8_t  _rx_head = 0;
    uint8_t  _rx_tail = 0;
};

/* 全局实例 — ISR 通过此指针访问 */
extern BspCan bsp_can;

#endif

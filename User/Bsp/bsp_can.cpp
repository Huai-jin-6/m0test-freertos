/**
 * @file bsp_can.cpp
 * @brief CAN 通信 BSP 层 — 非阻塞发送 + 环形接收缓冲 + ISR
 */

#include "bsp_can.h"
#include "ti_msp_dl_config.h"
#include <string.h>

/* ---- 全局实例 ---- */
BspCan bsp_can;

/* ---- ISR ---- */
extern "C" void MCAN0_INST_IRQHandler(void)
{
    bsp_can.isr_handler();
}

/* ================================================================
 *  公开 API
 * ================================================================ */

void BspCan::init()
{
    _rx_head = 0;
    _rx_tail = 0;
}

void BspCan::enable_irq()
{
    NVIC_ClearPendingIRQ(MCAN0_INST_INT_IRQN);
    NVIC_EnableIRQ(MCAN0_INST_INT_IRQN);
}

void BspCan::disable_irq()
{
    NVIC_DisableIRQ(MCAN0_INST_INT_IRQN);
}

bool BspCan::send(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (DL_MCAN_getTxBufReqPend(MCAN0_INST) & 0x01U)
        return false;

    DL_MCAN_TxBufElement tx;
    memset(&tx, 0, sizeof(tx));
    tx.id  = (id & 0x7FFU) << 18U;
    tx.dlc = (len > 8) ? 8 : len;
    if (len > 0 && data) memcpy(tx.data, data, tx.dlc);

    DL_MCAN_writeMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_BUF, 0U, &tx);
    DL_MCAN_TXBufAddReq(MCAN0_INST, 0U);
    return true;
}

bool BspCan::recv_read(uint32_t *id, uint8_t *data, uint8_t *len)
{
    if (_rx_tail == _rx_head) return false;

    uint8_t idx = _rx_tail;
    *id  = _rx_id[idx];
    *len = _rx_len[idx];
    memcpy(data, _rx_data[idx], *len);
    _rx_tail = (idx + 1) % RX_BUF_SIZE;
    return true;
}

/* ================================================================
 *  ISR: FIFO → 环形缓冲
 * ================================================================ */

void BspCan::isr_handler()
{
    DL_MCAN_RxFIFOStatus st;
    st.num = DL_MCAN_RX_FIFO_NUM_0;
    DL_MCAN_getRxFIFOStatus(MCAN0_INST, &st);

    while (st.fillLvl > 0)
    {
        DL_MCAN_RxBufElement rx;
        DL_MCAN_readMsgRam(MCAN0_INST, DL_MCAN_MEM_TYPE_FIFO, 0,
                           DL_MCAN_RX_FIFO_NUM_0, &rx);
        DL_MCAN_writeRxFIFOAck(MCAN0_INST, DL_MCAN_RX_FIFO_NUM_0, st.getIdx);

        uint8_t idx = _rx_head;
        _rx_id[idx]  = (rx.id >> 18U) & 0x7FFU;
        _rx_len[idx] = (rx.dlc > 8) ? 8 : (uint8_t)rx.dlc;
        memcpy(_rx_data[idx], rx.data, _rx_len[idx]);
        _rx_head = (idx + 1) % RX_BUF_SIZE;

        st.num = DL_MCAN_RX_FIFO_NUM_0;
        DL_MCAN_getRxFIFOStatus(MCAN0_INST, &st);
    }

    uint32_t ir = DL_MCAN_getIntrStatus(MCAN0_INST);
    DL_MCAN_clearIntrStatus(MCAN0_INST, ir, DL_MCAN_INTR_SRC_MCAN_LINE_1);
}

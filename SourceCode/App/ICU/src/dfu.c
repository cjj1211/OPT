/*******************************************************************************
 * @file  dfu.c
 * @brief DFU
 * @copyright
 *        Copyright(C) 2025 NeuroXess, All rights reserved.
 * @par Changelog
 ******************************************************************************/

#include "dfu.h"
#include "public_api.h"
#include "nx_crc.h"
#include "G3_Protocol.h"
#include "G3_ProtocolImplUart.h"

#define LOG_MODULE_NAME OPT_DFU
LOG_MODULE_REGISTER(LOG_MODULE_NAME, LOG_LEVEL_INF);

K_SEM_DEFINE(sem_stm32_dfu, 0, 1);
volatile bool g_stm32_dfu_enter = false;
volatile bool g_stm32_talk = false;
volatile dfu_type_t g_dfu_type = DFU_NONE;
volatile uint8_t dfu_data[STM32_FW_MAX_SIZE] = {0};

K_MSGQ_DEFINE(msgq_dfu_uart_rx, sizeof(struct msgq_uart_ack_t), 1, 4);
K_MSGQ_DEFINE(msgq_uart_rx, sizeof(struct msgq_uart_ack_t), 1, 4);
K_MSGQ_DEFINE(msgq_sync_uart_rx, sizeof(struct msgq_uart_ack_t), 1, 4);

void stm32_dfu_thread(void)
{
    uint16_t send_len = 0;
    uint8_t send_err = 0;

#if defined(CONFIG_THREAD_NAME)
    k_tid_t current_thread = k_current_get();
    printk("Thread ID: %p, Name: %s (prio: %d)\n", current_thread, current_thread->name, k_thread_priority_get(current_thread));
#endif

    for (;;)
    {
        k_sem_take(&sem_stm32_dfu, K_FOREVER);
        g_stm32_dfu_enter = true;
        dfu_internal_state_t st = DFU_BTR_ENTER;
        send_len = 0;
        send_err = 0;
        struct msgq_uart_ack_t ele;

        uint16_t dfu_crc = nx_crc16(dfu_data, sizeof(dfu_data));
        LOG_INF("DFU CRC = 0x%04x", dfu_crc);

        while (st != DFU_FINISH)
        {
            switch (st) {
            case DFU_BTR_ENTER:
            {
                LOG_INF("Step 1: Change to BTR");
                //!< Step 1: Notify STM32 to enter BTR mode and confirm

                uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
                Frame *frmAck = (Frame *)data;
                frmAck->header = HEADER;
                frmAck->seq = getBleSendSeq();
                frmAck->type = FRAME_UPLOAD_COMMAND;
                frmAck->dataLen = 1;
                frmAck->data[0] = UPGRADE_STARTED;
                ProtocolErrorCode ret = sendFrame(frmAck);

                do {
                    uint8_t data[MAX_UART_FRAME_SIZE] = {0};
                    Frame *p_uartframe = (Frame *)data;
                    p_uartframe->header = HEADER;
                    p_uartframe->type = FRAME_MODE_CHANGE;
                    p_uartframe->dataLen = 1;
                    p_uartframe->data[0] = 0x00;
                    sendUartFrame(p_uartframe);
                    if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0) {}

                    p_uartframe->type = FRAME_MODE_GET;
                    p_uartframe->dataLen = 0;
                    sendUartFrame(p_uartframe);
                    if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0) {
                        LOG_HEXDUMP_DBG(ele.data, ele.len, "DFU");
                        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
                        uint8_t type = ele.data[4];
                        uint16_t mode = ele.data[9];
                        if ((ack_head == HEADER) && (type == FRAME_MODE_GET) && (mode == 0)) {
                            send_err = 0;
                            st = DFU_METAINFO_SEND;
                        }
                        else
                        {
                            LOG_INF("FRAME_MODE_GET: %d", mode);
                            if (++send_err >= 5)
                            {
                                LOG_INF("Change to BTR mode err");
                                st = DFU_FAIL;
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (++send_err >= 5)
                        {
                            LOG_INF("Change to BTR mode err");
                            st = DFU_FAIL;
                            break;
                        }
                    }
                } while (st == DFU_BTR_ENTER);
            }
                break;
            case DFU_METAINFO_SEND:
            {
                LOG_INF("Step 2: Write Metainfo");
                //!< Step 2: Send meta info of firmware
                do {
                    uint8_t data[MAX_UART_FRAME_SIZE] = {0};
                    Frame *p_uartframe = (Frame *)data;
                    p_uartframe->header = HEADER;
                    p_uartframe->type = FRAME_WRITE;
                    p_uartframe->dataLen = 8;
                    p_uartframe->data[0] = 0xe0;
                    p_uartframe->data[1] = 0x1f;
                    p_uartframe->data[2] = 0x04;
                    p_uartframe->data[3] = 0x00;
                    p_uartframe->data[4] = (uint8_t)(dfu_crc);
                    p_uartframe->data[5] = (uint8_t)(dfu_crc >> 8);
                    sendUartFrame(p_uartframe);
                    if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0) {
                        LOG_HEXDUMP_DBG(ele.data, ele.len, "DFU");
                        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
                        uint8_t type = ele.data[4];
                        uint16_t len = ele.data[6] | (ele.data[7] << 8);
                        if ((ack_head == HEADER) && (type == FRAME_WRITE) && (len == 0)) {
                            send_err = 0;
                            send_len = 0;
                            st = DFU_BODY_SEND;
                        }
                        else {
                            if (++send_err >= 5)
                            {
                                LOG_INF("Send Metainfo err");
                                st = DFU_FAIL;
                                break;
                            }
                        }
                    }
                    else
                    {
                        if (++send_err >= 5)
                        {
                            LOG_INF("Send Metainfo err");
                            st = DFU_FAIL;
                            break;
                        }
                    }
                } while(st == DFU_METAINFO_SEND);
            }
                break;
            case DFU_BODY_SEND:
            {
                //!< Step 3: send body of firmware file
                while (send_len < sizeof(dfu_data))
                {
                    uint8_t data[MAX_UART_FRAME_SIZE] = {0};
                    Frame *p_uartframe = (Frame *)data;
                    p_uartframe->header = HEADER;
                    p_uartframe->type = FRAME_WRITE;
                    p_uartframe->dataLen = 132;
                    p_uartframe->data[0] = (uint8_t)(0x2000 + send_len);
                    p_uartframe->data[1] = (uint8_t)((0x2000 + send_len) >> 8);
                    // data len
                    p_uartframe->data[2] = 0x80;
                    p_uartframe->data[3] = 0x00;
                    memcpy(p_uartframe->data + 4, dfu_data + send_len, 0x80);
                    sendUartFrame(p_uartframe);
                    // wait ack
                    if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0)
                    {
                        uint16_t ack_head = ele.data[0] | (ele.data[1] << 8);
                        uint8_t type = ele.data[4];
                        uint16_t len = ele.data[6] | (ele.data[7] << 8);
                        if ((ack_head == HEADER) && (type == FRAME_WRITE) && (len == 0))
                        {
                            send_len += 0x80;
                            LOG_INF("DFU process: %d/%d, %d%%", send_len, sizeof(dfu_data), send_len * 100 / sizeof(dfu_data));
                        }
                        else
                        {
                            LOG_INF("DFU ack fail, 0x%04x", send_len);
                            if (++send_err >= 5)
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        LOG_INF("DFU ack timeout, 0x%04x, 0x%02x-0x%02x", send_len,  p_uartframe->data[0],  p_uartframe->data[1]);
                        if (++send_err >= 5)
                        {
                            break;
                        }
                    }
                }

                if (send_len >= sizeof(dfu_data))
                {
                    send_err = 0;
                    st = DFU_APP_ENTER;
                    LOG_INF("STM32 DFU finsih");
                }
                else
                {
                    send_err = 0;
                    st = DFU_FAIL;
                    LOG_INF("STM32 DFU fail");
                }
            }
                break;
            case DFU_APP_ENTER:
            {
                LOG_INF("Step 4: Change to APP");
                // uint8_t data[MAX_UART_FRAME_SIZE] = {0};
                // Frame *p_uartframe = (Frame *)data;
                // p_uartframe->header = HEADER;
                // p_uartframe->type = FRAME_MODE_CHANGE;
                // p_uartframe->dataLen = 1;
                // p_uartframe->data[0] = 0x01;
                // sendUartFrame(p_uartframe);
                // if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0) {
                // }
                st = DFU_SUCCESS;
            }
                break;
            case DFU_SUCCESS:
            {
                LOG_INF("STM32 DFU SUCCESS!");
                st = DFU_FINISH;

                uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
                Frame *frmAck = (Frame *)data;
                frmAck->header = HEADER;
                frmAck->seq = getBleSendSeq();
                frmAck->type = FRAME_UPLOAD_COMMAND;
                frmAck->dataLen = 1;
                frmAck->data[0] = UPGRADE_SUCCESS;
                sendFrame(frmAck);
                k_sleep(K_MSEC(100));
                sendFrame(frmAck);
                k_sleep(K_MSEC(100));

                uint8_t uart_data[MAX_UART_FRAME_SIZE] = {0};
                Frame *p_uartframe = (Frame *)uart_data;
                p_uartframe->header = HEADER;
                p_uartframe->type = FRAME_MODE_CHANGE;
                p_uartframe->dataLen = 1;
                p_uartframe->data[0] = 0x01;
                sendUartFrame(p_uartframe);
                if (k_msgq_get(&msgq_dfu_uart_rx, &ele, K_MSEC(1000)) == 0) {
                }
            }
                break;
            case DFU_FAIL:
            {
                LOG_INF("STM32 DFU FAIL!");
                st = DFU_FINISH;

                uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
                Frame *frmAck = (Frame *)data;
                frmAck->header = HEADER;
                frmAck->seq = getBleSendSeq();
                frmAck->type = FRAME_UPLOAD_COMMAND;
                frmAck->dataLen = 1;
                frmAck->data[0] = UPGRADE_FAILED;
                sendFrame(frmAck);
                k_sleep(K_MSEC(100));
                sendFrame(frmAck);
            }
                break;
            default:
                st = DFU_FINISH;
                break;
            }
        }

        g_stm32_dfu_enter = false;
        k_timer_start(&stm_heartbeat_timer, K_MSEC(STM_HB_TIMER_MS), K_MSEC(STM_HB_TIMER_MS));
    }
}

K_THREAD_DEFINE(stm32_dfu_thread_id, 2048, stm32_dfu_thread, NULL, NULL,
        NULL, STM32_DFU_PRIORITY, 0, 1000);

/**
  * @file protocol.c
  * @author DCX (chunxiao.dai@neuroxess.com)
  * @brief protocol parser
  * @version 0.1
  * @date 2025-01-15
  * 
  * @copyright Copyright (C) 2025 NeuroXess
  * 
  */

#include "protocol.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "flash.h"
#include "crc16.h"

extern volatile uint8_t g_run_mode_entry;
extern fw_info_t btr_info;

/******************************************************************************
 * @defgroup Variables 
 *****************************************************************************/
volatile uint16_t g_frame_timeout = 0;
volatile bool g_rx_recv_flag = false;

sdu_ctx_t g_rx_buf = {{0}, SDU_WAIT_HEAD_LSB, 0};

/**
 * @brief Calculate the actual length of SDU
 * 
 * @param p_sdu
 * @return uint8_t 
 */
uint8_t sdu_length(const sdu_t *p_sdu)
{
    return sizeof(*p_sdu) - sizeof(p_sdu->pdu.data) + p_sdu->pdu.len;
}

/**
 * @brief Calculate the checksum of SDU
 * @param[in] pSDU
 */
void sdu_checksum (sdu_t *p_sdu)
{
    p_sdu->checksum = 0;

    for (int i = 0; i < sdu_length(p_sdu) - 1; i++)
    {
        p_sdu->checksum += *(((uint8_t*)p_sdu) + i);
    }
}

/**
 * @brief Initialize SDU context
 * 
 * @param p_sdu_ctx 
 */
void sdu_ctx_init(sdu_ctx_t *p_sdu_ctx)
{
    uint8_t *p = (uint8_t*)p_sdu_ctx;

    for (int i = 0; i < sizeof(p_sdu_ctx->sdu); i++)
    {
        *p++ = 0;
    }
    p_sdu_ctx->index = 0;
    p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
}

/**
 * @brief Send SDU packet by uart
 * @param[in] cmd - command
 * @param[in] sz  - datasize
 * @param[in] *data
 */
void sdu_send (uint8_t cmd, void * data, uint8_t sz)
{
    sdu_t sdu = {0};

    /// sdu construct
    sdu.sync_lsb = SDU_SYNC_HEAD_LSB;
    sdu.sync_msb = SDU_SYNC_HEAD_MSB;
    sdu.seq = 0;
    sdu.pdu.type = cmd;
    sdu.pdu.len = sz;
    memcpy(&sdu.pdu.data, data, sz);
    sdu_checksum(&sdu);

    /// actual transfer function
    USART2_SendData((uint8_t*)&sdu, sdu_length(&sdu) - 1);
    USART2_SendData((uint8_t*)&sdu.checksum, sizeof(sdu.checksum));
}

/**
 * @brief SDU command reply
 */
void sdu_cmd_ack(uint8_t cmd)
{
    sdu_send(cmd, NULL, 0);
}

/**
 * @brief SDU command reply with flag
 */
void sdu_cmd_ack_flag (uint8_t cmd, uint8_t flag)
{
    uint8_t data[2] = {0x00, flag};
    sdu_send(cmd, data, sizeof(data));
}

/**
 * @brief SDU command reply error
 */
void sdu_cmd_ack_err (uint8_t cmd, uint8_t err)
{
    uint8_t data[2] = {0x00, err};
    sdu_send(cmd, data, sizeof(data));
}

/**
 * @brief Serial communication protocol analysis function
 * 
 * @param p_sdu_ctx fill structure SDU context
 * @param byte received byte
 * @return true - get a valid sdu packet
 * @return false - reception process has not complete
 */
bool sdu_analyze(sdu_ctx_t *p_sdu_ctx, uint8_t byte)
{
    bool ret_val = false;

    if (g_frame_timeout == MSG_FRAME_RECEIVED)
    {
        return ret_val;
    }
    if (g_frame_timeout == 0)
    {
        p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
    }
    g_frame_timeout = MSG_FRAME_TIMEOUT;

    switch (p_sdu_ctx->state) {
    case SDU_WAIT_HEAD_LSB:
        if (byte != SDU_SYNC_HEAD_LSB) {
            break;
        }
        p_sdu_ctx->sdu.sync_lsb = byte;
        p_sdu_ctx->sdu.checksum = byte;
        p_sdu_ctx->state = SDU_WAIT_HEAD_MSB;
        break;
    case SDU_WAIT_HEAD_MSB:
        if (byte != SDU_SYNC_HEAD_MSB) {
            p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
            break;
        }
        p_sdu_ctx->sdu.sync_msb = byte;
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = SDU_WAIT_SEQ_LSB;
        break;
    case SDU_WAIT_SEQ_LSB:
        p_sdu_ctx->sdu.seq = byte;
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = SDU_WAIT_SEQ_MSB;
        break;
    case SDU_WAIT_SEQ_MSB:
        p_sdu_ctx->sdu.seq |= (byte << 8);
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = SDU_WAIT_CMD_TYPE;
        break;
    case SDU_WAIT_CMD_TYPE:
        p_sdu_ctx->sdu.pdu.type = byte;
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = SDU_WAIT_LEN_LSB;
        break;
    case SDU_WAIT_LEN_LSB:
        p_sdu_ctx->sdu.pdu.len = byte;
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = SDU_WAIT_LEN_MSB;
        break;
    case SDU_WAIT_LEN_MSB:
        p_sdu_ctx->sdu.pdu.len |= (byte << 8);
        if (p_sdu_ctx->sdu.pdu.len > PDU_DATA_SIZE) {
            p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
            break;
        }
        p_sdu_ctx->index = 0;
        p_sdu_ctx->sdu.checksum += byte;
        p_sdu_ctx->state = p_sdu_ctx->sdu.pdu.len ? SDU_WAIT_DATABODY : SDU_WAIT_CHECKSUM;
        break;
    case SDU_WAIT_DATABODY:
        p_sdu_ctx->sdu.pdu.data[p_sdu_ctx->index++] = byte;
        p_sdu_ctx->sdu.checksum += byte;
        if (p_sdu_ctx->index >= p_sdu_ctx->sdu.pdu.len)
        {
            p_sdu_ctx->state = SDU_WAIT_CHECKSUM;
        }
        break;
    case SDU_WAIT_CHECKSUM:
        if (p_sdu_ctx->sdu.checksum == byte)
        {
            p_sdu_ctx->state = SDU_RECV_FINISHED;
            g_frame_timeout = MSG_FRAME_RECEIVED;
            ret_val = true;
        }
        else
        {
            p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
        }
        break;
    case SDU_RECV_FINISHED:
        break;
    default:
        p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
        break;
    }

    return ret_val;
}

/**
 * @brief Parse protocol data
 * 
 * @param p_sdu_ctx 
 */
void protocol_data_parse(sdu_ctx_t *p_sdu_ctx)
{
    sdu_ctx_t l_sdu_ctx = *p_sdu_ctx;
    pdu_t *p_pdu = &l_sdu_ctx.sdu.pdu;
    sdu_ctx_init(p_sdu_ctx);

    uint16_t command = p_pdu->type;

    if (SDU_RECV_FINISHED == l_sdu_ctx.state)
    {
        switch (command) {
        case FRAME_MODE_CHANGE:
            if (p_pdu->data[0] == 0x01)
            {
                if (crc16((void*)APP_LOWER, APP_SIZE) == *((uint16_t*)BTR_DATA))
                {
                    CPU_SwitchToApp();
                }
                else
                {
                    sdu_cmd_ack_err(FRAME_MODE_CHANGE, 5);
                }
            }
            break;
        case FRAME_MODE_GET:
//            printf("mode is %d\n", g_run_mode_entry);
            sdu_cmd_ack_flag(FRAME_MODE_GET, g_run_mode_entry);
            break;
        case FRAME_VERSION_GET:
            {
                uint8_t data[5] = {0};
                data[0] = RUN_MODE_BTR;
                data[1] = btr_info.x;
                data[2] = btr_info.y;
                data[3] = btr_info.z;
                data[4] = btr_info.b;
                sdu_send(FRAME_VERSION_GET, data, sizeof(data));
            }
            break;
        case FRAME_READ:
            {
                uint32_t addr =  FLASH_BASE + (p_pdu->data[0] | (p_pdu->data[1] << 8));
                uint16_t len = p_pdu->data[2] | (p_pdu->data[3]);
//                printf("addr = 0x%08x, len = %d\n", addr, len);
                uint32_t data[2] = {0x01020304, 0x05060708};
                STM32_Flash_Read(addr, data, len);
                sdu_send(FRAME_READ, (uint8_t*)data, sizeof(data));
            }
            break;
        case FRAME_WRITE:
            {
                bool err = true;
                uint32_t addr =  FLASH_BASE + (p_pdu->data[0] | (p_pdu->data[1] << 8));
                uint16_t len = p_pdu->data[2] | (p_pdu->data[3]);
                uint32_t *data = (uint32_t*)(p_pdu->data + 4);

                err = STM32_Flash_Write(addr, data, len / 4);
                if (err)
                    sdu_cmd_ack(FRAME_WRITE);
                else
                    sdu_cmd_ack_err(FRAME_WRITE, 1);
            }
            break;
        case FRAME_APP_CRC:
            {
                uint16_t crc_value = crc16((void*)APP_LOWER, APP_SIZE);
                sdu_send(FRAME_APP_CRC, (uint8_t*)&crc_value, sizeof(crc_value));
            }
            break;
        default:
            break;
        }
    }
}

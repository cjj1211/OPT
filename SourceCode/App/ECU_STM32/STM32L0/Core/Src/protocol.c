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
#include "usart.h"
#include "hmi.h"
#include <stdio.h>
#include <string.h>


/******************************************************************************
 * @defgroup Variables 
 *****************************************************************************/
volatile uint16_t g_frame_timeout = 0;
volatile bool g_rx_recv_flag = false;

sdu_ctx_t g_rx_buf = {{0}, SDU_SYNC_HEAD_MSB, 0};

pdu_handler ecu_pdu_handlers[SDU_HANDLER_NUM] = {NULL};

void handler_charge_state_notify(pdu_t *p_pdu)
{
    switch (p_pdu->data[0])
    {
        case ECU_PollCharging:
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            break;
        case ECU_PollFieldOff:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
            break;
        case ECU_PollWait:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_TogglePin(LED_Y_GPIO_Port, LED_Y_Pin);
        default:
            break;
    }
}

void handler_led_ctrl(pdu_t *p_pdu)
{
    switch (p_pdu->data[0])
    {
        case LED_Y:
            HAL_GPIO_TogglePin(LED_Y_GPIO_Port, LED_Y_Pin);
            break;
        case LED_G:
            HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
            break;
        case LED_R:
            HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
            break;
        default:
            break;
    }
}

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
    p_sdu_ctx->state = SDU_SYNC_HEAD_LSB;
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
        p_sdu_ctx->state = SDU_WAIT_HEAD_MSB;
        break;
    case SDU_WAIT_HEAD_MSB:
        if (byte != SDU_SYNC_HEAD_MSB) {
            p_sdu_ctx->state = SDU_SYNC_HEAD_LSB;
            break;
        }
        p_sdu_ctx->sdu.sync_msb = byte;
        p_sdu_ctx->state = SDU_WAIT_SEQ_LSB;
        break;
    case SDU_WAIT_SEQ_LSB:
        p_sdu_ctx->sdu.seq = byte;
        p_sdu_ctx->state = SDU_WAIT_SEQ_MSB;
        break;
    case SDU_WAIT_SEQ_MSB:
        p_sdu_ctx->sdu.seq |= (byte << 8);
        p_sdu_ctx->state = SDU_WAIT_CMD_TYPE;
        break;
    case SDU_WAIT_CMD_TYPE:
        p_sdu_ctx->sdu.pdu.type = byte;
        p_sdu_ctx->state = SDU_WAIT_LEN_LSB;
        break;
    case SDU_WAIT_LEN_LSB:
        p_sdu_ctx->sdu.pdu.len = byte;
        p_sdu_ctx->state = SDU_WAIT_LEN_MSB;
        break;
    case SDU_WAIT_LEN_MSB:
        p_sdu_ctx->sdu.pdu.len |= (byte << 8);
        if (p_sdu_ctx->sdu.pdu.len > PDU_DATA_SIZE) {
            p_sdu_ctx->state = SDU_WAIT_HEAD_LSB;
            break;
        }
        p_sdu_ctx->index = 0;
        p_sdu_ctx->state = p_sdu_ctx->sdu.pdu.len ? SDU_WAIT_DATABODY : SDU_WAIT_FRAMEEND;
        break;
    case SDU_WAIT_DATABODY:
        p_sdu_ctx->sdu.pdu.data[p_sdu_ctx->index++] = byte;
        if (p_sdu_ctx->index >= p_sdu_ctx->sdu.pdu.len)
        {
            p_sdu_ctx->state = SDU_WAIT_FRAMEEND;
        }
        break;
    case SDU_WAIT_FRAMEEND:
        if (byte == SDU_SYNC_TAIL)
        {
            p_sdu_ctx->state = SDU_RECV_FINISHED;
            g_frame_timeout = MSG_FRAME_RECEIVED;
            ret_val = true;
        }
        else
        {
            p_sdu_ctx->state = SDU_SYNC_HEAD_LSB;
        }
        break;
    case SDU_RECV_FINISHED:
        break;
    default:
        p_sdu_ctx->state = SDU_SYNC_HEAD_LSB;
        break;
    }

    return ret_val;
}

void handlers_init(void)
{
    ecu_pdu_handlers[FRAME_CHARGESTATE] = handler_charge_state_notify;
    ecu_pdu_handlers[FRAME_LED] = handler_led_ctrl;
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

    if (SDU_RECV_FINISHED == l_sdu_ctx.state)
    {
        if (ecu_pdu_handlers[p_pdu->type] != NULL)
        {
            ecu_pdu_handlers[p_pdu->type](p_pdu);
        }
    }
}

void sdu_send(uint8_t type, uint16_t len, uint8_t *data)
{
    sdu_t sdu = {0xAA, 0x55, 0, {0}};

    sdu.pdu.type = type;
    sdu.pdu.len = len;
    memcpy(sdu.pdu.data, data, sdu.pdu.len);
    sdu.pdu.data[sdu.pdu.len] = 0xFF;
    sdu.pdu.len++;
    
    uart_send((uint8_t*)&sdu, sdu_length(&sdu));
}

void key_event_notify(ButtonStatus event)
{
    switch (event)
    {
      case OPENICU:
      {
          uint8_t data = OPENICU;
          sdu_send(FRAME_OPENICU, 1, &data);
      }
          break;
      case OPENECU:
      {
          uint8_t data = OPENECU;
          sdu_send(FRAME_OPENICU, 1, &data);
      }
          break;
      case CLOSEECU:
      {
          uint8_t data = CLOSEECU;
          sdu_send(FRAME_OPENICU, 1, &data);
      }
          break;
      case CLOSEICU:
      {
          uint8_t data = CLOSEICU;
          sdu_send(FRAME_OPENICU, 1, &data);
      }
          break;
      default:
          break;
    }
}
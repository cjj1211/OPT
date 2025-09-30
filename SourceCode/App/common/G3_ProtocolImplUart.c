/******************************************************************************
 *  版权所有（C）2022-2024，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  描述 : Uart接口协议实现
 *  修改记录:
 *
 ******************************************************************************/

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "G3_ProtocolImplUart.h"
#include "HardwareDefine.h"
#include "src/public_api.h"
#include <zephyr/drivers/uart.h>
#include "G3_seq.h"
#include <zephyr/kernel.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief 全局统一的Uart发送方帧序号
 *
 */
static uint16_t mSendSeq = 0;

/**
 * @brief 期望收到的下一个帧序号
 *
 */
static uint16_t mCurrentRcvUartSeq = 0;

FrameHandler UartFrameHandlers[FRAME_HANDLER_NUM] = {0};


void initUartProtocol(void)
{
}

void resetUartSendSeq(void)
{
    resetSendSeq(&mSendSeq);
}

void resetUartRcvSeq(void)
{
    resetRecvSeq(&mCurrentRcvUartSeq);
}

ProtocolErrorCode receiveUartFrame(uint8_t *p_data, uint16_t data_len)
{
    if (p_data == NULL || data_len == 0)
    {
        return ERROR_INVALID_PARAM;
    }
    Frame *p_frame = (Frame *)p_data;
    ProtocolErrorCode ret = dataToFrame(p_data, data_len, p_frame);
    if (ret != ERROR_NO_ERROR)
    {
      
        return ret;
    }
    int index = p_frame->type;
    FrameHandler handler = UartFrameHandlers[index];
    if (handler != NULL)
    {
        handler(p_frame);
    }
    else
    {
        
    }

    return ERROR_NO_ERROR;
}

uint8_t checksum(uint8_t *data, uint16_t len)
{
    uint8_t value = 0;

    for (int i = 0; i < len; i++)
    {
        value += data[i];
    }

    return value;
}

ProtocolErrorCode sendUartFrame(Frame *p_frame)
{
    uint8_t data[MAX_UART_FRAME_SIZE];
    uint16_t data_len = FRAME_HEADER_LEN + p_frame->dataLen;
    p_frame->seq = getSendSeq(&mSendSeq);
    memcpy(data, p_frame, FRAME_HEADER_LEN);
    memcpy(data + FRAME_HEADER_LEN, p_frame->data, p_frame->dataLen);
    // uint8_t sum = checksum(data, data_len);
    // printk("checksum = 0x%x, %d\n", sum, data_len);
    data[data_len] = checksum(data, data_len);
    data_len++;
    // for (int i = 0; i < data_len; i++)
    // {
    //     printk("0x%02x ", data[i]);
    // }
    // printk("\n");
    uart_tx(uart, data, data_len, SYS_FOREVER_MS);
    k_sem_take(&sem_uart_tx_done, K_MSEC(50));

    return ERROR_NO_ERROR;
}

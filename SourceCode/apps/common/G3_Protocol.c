/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋廷宇 tingyu.song@neuroxess.com
 *  描述 : 无线传输协议的公共实现
 *  修改记录:
 *
 ******************************************************************************/

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "nrf_log.h"

ProtocolErrorCode dataToFrame(uint8_t *p_data, uint16_t data_len, Frame *p_frame)
{
    if (p_data == NULL || data_len == 0 || p_frame == NULL)
    {
        return ERROR_INVALID_PARAM;
    }
    if (data_len < FRAME_HEADER_LEN)
    {
        return ERROR_INVALID_FRAME;
    }
    // p_frame = (Frame *)p_data;
    if (p_frame->header != HEADER)
    {
        return ERROR_INVALID_HEADER;
    }

    return ERROR_NO_ERROR;
}

ProtocolErrorCode frameToData(Frame *p_frame, uint8_t *p_data, uint16_t *data_len)
{
    if (p_frame == NULL || p_data == NULL || data_len == NULL)
    {
        return ERROR_INVALID_PARAM;
    }
    memcpy(p_data, p_frame, FRAME_HEADER_LEN);
    memcpy(p_data + FRAME_HEADER_LEN, p_frame->data, p_frame->dataLen);
    *data_len = FRAME_HEADER_LEN + p_frame->dataLen;
    *(p_data + *data_len) = 0x00;
    (*data_len)++;
    return ERROR_NO_ERROR;
}

ProtocolErrorCode deepFrameToData(Frame *p_frame, uint8_t *p_data, uint16_t *data_len)
{
    if (p_frame == NULL || p_data == NULL || data_len == NULL)
    {
        return ERROR_INVALID_PARAM;
    }

    // Calculate the total length of the Frame
    uint16_t frame_length = sizeof(Frame) - sizeof(p_frame->data) + p_frame->dataLen;
    uint16_t required_len = frame_length + 1; // +1 for the null byte

    if (*data_len < required_len)
    {
        return ERROR_INVALID_PARAM;
    }

    // Copy the Frame header and data to p_data
    memcpy(p_data, p_frame, frame_length);

    // Update the data length
    *data_len = frame_length;

    // Append the null byte
    p_data[*data_len] = 0x00;
    (*data_len)++;

    return ERROR_NO_ERROR;
}
void frmHandlerHeartbeat(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv Heartbeat Frame");
}

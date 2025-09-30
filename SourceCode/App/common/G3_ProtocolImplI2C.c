/******************************************************************************
 *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  描述 : I2C接口协议实现
 *  修改记录:
 *
 ******************************************************************************/

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "G3_ProtocolImplI2C.h"
#include "HardwareDefine.h"
#include "G3_seq.h"

/**
 * @brief 全局统一的I2C发送方帧序号
 *
 */
static uint16_t mSendSeq = 0;

/**
 * @brief 期望收到的下一个帧序号
 *
 */
static uint16_t mCurrentRcvI2CSeq = 0;

FrameHandler I2CFrameHandlers[FRAME_HANDLER_NUM] = {0};

void initI2CProtocol(void)
{
}

void resetI2CSendSeq(void)
{
 
    resetSendSeq(&mSendSeq);
  
}

void resetI2CRcvSeq(void)
{
 
    resetRecvSeq(&mCurrentRcvI2CSeq);

}

ProtocolErrorCode receiveI2CFrame(uint8_t *p_data, uint16_t data_len)
{

    // if (p_data == NULL || data_len == 0)
    // {
    //     return ERROR_INVALID_PARAM;
    // }
    // Frame *p_frame = (Frame *)p_data;
    // ProtocolErrorCode ret = dataToFrame(p_data, data_len, p_frame);
    // if (ret != ERROR_NO_ERROR)
    // {
    //     NRF_LOG_WARNING("Recv I2C Frame ERROR: %d", ret);
    //     return ret;
    // }
    // int index = p_frame->type;
    // FrameHandler handler = I2CFrameHandlers[index];
    // if (handler != NULL)
    // {
    //     handler(p_frame);
    // }
    // else
    // {
    //     NRF_LOG_WARNING("Recved I2C Frame Type: %d, but no handler", p_frame->type);
    // }

    return ERROR_NO_ERROR;
}

ProtocolErrorCode writeFrame(uint8_t slaveAddress, uint8_t reg_address, Frame *p_frame)
{
}
ProtocolErrorCode readFrame(uint8_t slaveAddress, uint8_t reg_address, Frame *p_frame)
{
}
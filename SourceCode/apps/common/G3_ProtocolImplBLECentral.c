/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋廷宇 tingyu.song@neuroxess.com
 *  描述 : 无线传输协议的主机实现
 *  修改记录:
 *
 ******************************************************************************/

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "G3_ProtocolUSB.h"
#include "G3_Seq.h"
#include "nrf_log.h"
#include "ble_nus_c.h"
#include "extern_resources.h"

/**
 * @brief 全局统一的发送方帧序号
 *
 */
static uint16_t bleSendSeq = 0;

/**
 * @brief 全局统一的BLE接收帧序号
 *
 */
static uint16_t mCurrentBleRcvSeq = 0;

uint16_t getBleSendSeq()
{
    return getSendSeq(&bleSendSeq);
}

void resetBleSendSeq(void)
{
    resetSendSeq(&bleSendSeq);
}

void resetBleRcvSeq(void)
{
    resetRecvSeq(&mCurrentBleRcvSeq);
}

FrameHandler bleFrameHandlers[FRAME_HANDLER_NUM] = {NULL};

void initProtocol(void)
{
    bleFrameHandlers[FRAME_HEARTBEAT] = frmHandlerHeartbeat;
    bleFrameHandlers[FRAME_ICU_STATUS] = frmHandlerICUStatus;
    bleFrameHandlers[FRAME_SET_WIRE_IN_VALUE] = frmHandlerSetWireInValue;
    bleFrameHandlers[FRAME_UPDATE_WIRE_IN] = frmHandlerUpdateWireIn;
    bleFrameHandlers[FRAME_GET_WIRE_IN_VALUE] = frmHandlerGetWireInValue;
    bleFrameHandlers[FRAME_UPDATE_WIRE_OUT] = frmHandlerUpdateWireOut;
    bleFrameHandlers[FRAME_GET_WIRE_OUT_VALUE] = frmHandlerGetWireOutValue;
		bleFrameHandlers[FRAME_ACT_TRIGGER_IN] = frmHandlerActivateTriggerIn;
	  bleFrameHandlers[FRAME_UPDATE_TRIGGER_OUT] = frmHandlerUpdateTriggerOuts;
	  bleFrameHandlers[FRAME_IS_TRIGGERED] = frmHandlerIsTriggered;
    bleFrameHandlers[FRAME_UPLOAD_COMMAND] = frmHandlerUploadCommond;
	 bleFrameHandlers[FRAME_WRITE_BLOCK_DATA] = frmHandlerReadBlockData;
}
static void frmHandlerReadBlockData(Frame *frame)
{
    NRF_LOG_INFO("Recv frmHandlerReadBlockData BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("frmHandlerReadBlockData report TO USB FAILED");
    }
}
ProtocolErrorCode receiveFrame(uint8_t *p_data, uint16_t data_len)
{
    if (p_data == NULL || data_len == 0)
    {
        return ERROR_INVALID_PARAM;
    }
    Frame *p_frame = (Frame *)p_data;
    ProtocolErrorCode ret = dataToFrame(p_data, data_len, p_frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_INFO("Recv BLE Frame ERROR: %d", ret);
        return ret;
    }

    if (p_frame->type == FRAME_HEARTBEAT)
    {
        return ERROR_NO_ERROR;
    }

    //    if(!checkRecvSeqIsValid(&mCurrentBleRcvSeq, p_frame->seq)){
    //        NRF_LOG_WARNING("Recved BLE Frame Seq: %d, but wait for: %d", p_frame->seq, mCurrentBleRcvSeq);
    //        return ERROR_INVALID_SEQ;
    //    }

    int index = p_frame->type;
    FrameHandler handler = bleFrameHandlers[index];
		NRF_LOG_INFO("index is %d",index);
    if (handler != NULL)
    {
        handler(p_frame);
    }
    else
    {
        NRF_LOG_INFO("Recved BLE Frame Type: %d, but no handler", p_frame->type);
    }

    return ERROR_NO_ERROR;
}

ProtocolErrorCode sendFrame(void *bleService, Frame *p_frame)
{
    uint8_t data[WIRELESS_BUF_SIZE];
    uint16_t data_len = 0;
    p_frame->seq = getBleSendSeq();
    frameToData(p_frame, data, &data_len);
    ble_nus_c_t *m_nus = (ble_nus_c_t *)bleService;
    int32_t ret = ble_nus_c_string_send(m_nus, data, data_len);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Send BLE Frame ERROR: %d", ret);
        return ERROR_SEND_FAILED;
    }

    // NRF_LOG_DEBUG("Sent BLE data to ICU:");
    // NRF_LOG_HEXDUMP_DEBUG(data,data_len);
    return ERROR_NO_ERROR;
}

bool isBLEConnected(void)
{
    return m_ble_connected;
}

static void frmHandlerICUStatus(Frame *frame)
{
    NRF_LOG_INFO("Recv ICU Status BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("ICU Status report TO USB FAILED");
    }
}
static void frmHandlerUploadCommond(Frame *frame)
 {
    NRF_LOG_INFO("-----------------Recv Handler Upload Commond BLE Frame---------------------------------:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Handler Upload Commond report TO USB FAILED");
    }
}
static void frmHandlerSetWireInValue(Frame *frame)
{
    NRF_LOG_INFO("Recv Set Wire In Value BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Wire In Value report TO USB FAILED");
    }
}
static void frmHandlerGetWireInValue(Frame *frame)
{
    NRF_LOG_INFO("Recv Get  Wire In Value BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Get Wire In Value report TO USB FAILED");
    }
}
static void frmHandlerUpdateWireOut(Frame *frame)
{
    NRF_LOG_INFO("Recv Update Wire Out BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Update Wire Out report TO USB FAILED");
    }
}
static void frmHandlerGetWireOutValue(Frame *frame)
{
    NRF_LOG_INFO("Recv Get Wire Out Value BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Get Wire Out Valu report TO USB FAILED");
    }
}
static void frmHandlerUpdateWireIn(Frame *frame)
{
    NRF_LOG_INFO("Recv Update Wire In  BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Update Wire In report TO USB FAILED");
    }
}

static void frmHandlerActivateTriggerIn(Frame *frame)
{
    NRF_LOG_INFO("Recv Activate Trigger In Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Activate Trigger In  report TO USB FAILED");
    }
}

static void frmHandlerUpdateTriggerOuts(Frame *frame)
{
    NRF_LOG_INFO("Recv Update Trigger Outs BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);
    NRF_LOG_INFO("index: %d", frame->data[0]);
    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Update Trigger Outs report TO USB FAILED");
    }
}

static void frmHandlerIsTriggered(Frame *frame)
{
    NRF_LOG_INFO("Recv Is Triggered BLE Frame:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);

    ProtocolErrorCode ret = sendUSBFrame(frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Is Triggered report TO USB FAILED");
    }
}



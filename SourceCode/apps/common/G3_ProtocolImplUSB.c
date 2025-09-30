/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋廷宇 tingyu.song@neuroxess.com
 *  描述 : USB接口协议实现
 *  修改记录:
 *
 ******************************************************************************/

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "G3_ProtocolUSB.h"
#include "HardwareDefine.h"
#include "nrf_log.h"
#include "extern_resources.h"
#include "app_usbd_cdc_acm.h"
#include "G3_seq.h"

/**
 * @brief 全局统一的USB发送方帧序号
 *
 */
static uint16_t mSendSeq = 0;

/**
 * @brief 期望收到的下一个帧序号
 *
 */
static uint16_t mCurrentRcvUSBSeq = 0;

FrameHandler usbFrameHandlers[FRAME_HANDLER_NUM] = {NULL};

void initUSBProtocol(void)
{
    usbFrameHandlers[FRAME_ICU_STATUS] = frmHandlerICUStatus;
    usbFrameHandlers[FRAME_DEVICE_INFO] = frmHandlerDeviceInfo;
    usbFrameHandlers[FRAME_DEVICE_MODEL] = frmHandlerDeviceModelList;
    usbFrameHandlers[FRAME_SET_WIRE_IN_VALUE] = frmHandlerSetWireInValue;
    usbFrameHandlers[FRAME_UPDATE_WIRE_IN] = frmHandlerUpdateWireIn;
    usbFrameHandlers[FRAME_GET_WIRE_IN_VALUE] = frmHandlerGetWireInValue;
    usbFrameHandlers[FRAME_UPDATE_WIRE_OUT] = frmHandlerUpdateWireOut;
    usbFrameHandlers[FRAME_GET_WIRE_OUT_VALUE] = frmHandlerGetWireOutValue;
	  usbFrameHandlers[FRAME_ACT_TRIGGER_IN] = frmHandlerActivateTriggerIn;
	  usbFrameHandlers[FRAME_UPDATE_TRIGGER_OUT] = frmHandlerUpdateTriggerOuts;
	  usbFrameHandlers[FRAME_IS_TRIGGERED] = frmHandlerIsTriggered;
		usbFrameHandlers[FRAME_UPLOAD_COMMAND] = frmHandlerUploadCommond;
	  usbFrameHandlers[FRAME_READ_BLOCK_DATA] = frmHandlerReadBlockData;	
}

static void frmHandlerReadBlockData (Frame *p_frame)
{
	
	   NRF_LOG_INFO("Recv frmHandlerReadBlockData query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  frmHandlerReadBlockData In query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerUploadCommond (Frame *p_frame)
{
	
	   NRF_LOG_INFO("--------------------Recv Upload Commond query Frame from USB.---------------------------------------------");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  frmHandlerUploadCommond query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerActivateTriggerIn(Frame * p_frame)
{
    NRF_LOG_INFO("Recv Activate Trigger In query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  Activate Trigger In query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerUpdateTriggerOuts(Frame * p_frame)
{
	NRF_LOG_INFO("Recv Update Trigger Outs query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  SUpdate Trigger Outs query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerIsTriggered(Frame * p_frame)
{

	NRF_LOG_INFO("Recv Is Triggered query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  Is Triggered query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerSetWireInValue(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Set Wire In Value query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  Set Wire In Value query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}
static void frmHandlerUpdateWireIn(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Update Wire In query Frame from USB.");
    if (isBLEConnected())
    {
         sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}
static void frmHandlerGetWireInValue(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Get Wire In Value query Frame from USB.");
    if (isBLEConnected())
    {
			  NRF_LOG_INFO("Send BLE  Get  Wire In Value query Frame .");
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}
static void frmHandlerUpdateWireOut(Frame *p_frame)
{

    NRF_LOG_INFO("Recv Update Wire Out query Frame from USB.");
    if (isBLEConnected())
    {
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}
static void frmHandlerGetWireOutValue(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Get Wire Out Value query Frame from USB.");
    if (isBLEConnected())
    {
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerDeviceModelList(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Set Wire In Value query Frame from USB.");
    if (isBLEConnected())
    {
         sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

static void frmHandlerDeviceInfo(Frame *p_frame)
{
    NRF_LOG_INFO("Recv Device Info query Frame from USB.");
    if (isBLEConnected())
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 0x0061;
        char icuModel = ECU_MODE;
        char icuId[32] = "NXG3ECU123456789";
        char firmwareVersion[32] = "1.0.0";
        char hdlVersion[32] = "1.0.0";
        uint8_t data[0x0061];
        int index = 0;
        data[0] = icuModel;
        index++;
        NRF_LOG_INFO("copy icuModel .");
        memcpy(data + index, icuId, sizeof(icuId));
        NRF_LOG_INFO("copy icuId .");
        index += sizeof(icuId);
        memcpy(data + index, firmwareVersion, sizeof(firmwareVersion));
        NRF_LOG_INFO("copy firmwareVersion .");
        index += sizeof(firmwareVersion);
        memcpy(data + index, hdlVersion, sizeof(hdlVersion));
        NRF_LOG_INFO("copy hdlVersion .");
        sendUSBLongFrame(&frame, data);
        NRF_LOG_INFO("sendUSBLongFrame .");
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

void resetUSBSendSeq(void)
{
    uint16_t old = mSendSeq;
    resetSendSeq(&mSendSeq);
    NRF_LOG_DEBUG("=====RESET USB SEND SEQ:==========old:new=%d:%d", old, mSendSeq);
}

void resetUSBRcvSeq(void)
{
    uint16_t old = mCurrentRcvUSBSeq;
    resetRecvSeq(&mCurrentRcvUSBSeq);
    NRF_LOG_DEBUG("=====RESET USB RECV SEQ:==========old:new=%d:%d", old, mCurrentRcvUSBSeq);
}

static void frmHandlerICUStatus(Frame *p_frame)
{
    NRF_LOG_INFO("Recv ICU Status query Frame from USB.");
    if (isBLEConnected())
    {
        sendFrame(getBLE_NUS_C(), p_frame);
    }
    else
    {
        Frame frame;
        frame.header = HEADER;
        frame.seq = getSendSeq(&mSendSeq);
        frame.type = p_frame->type;
        frame.dataLen = 1;
        frame.data[0] = ICU_OFFLINE;
        sendUSBFrame(&frame);
    }
}

ProtocolErrorCode receiveUSBFrame(uint8_t *p_data, uint16_t data_len)
{

    if (p_data == NULL || data_len == 0)
    {
        return ERROR_INVALID_PARAM;
    }
    Frame *p_frame = (Frame *)p_data;
    ProtocolErrorCode ret = dataToFrame(p_data, data_len, p_frame);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_WARNING("Recv USB Frame ERROR: %d", ret);
        return ret;
    }

    //    if (!checkRecvSeqIsValid(&mCurrentRcvUSBSeq, p_frame->seq)){
    //       NRF_LOG_WARNING("Recved USB Frame Seq: %d, but wait for: %d", p_frame->seq, mCurrentRcvUSBSeq);
    //       return ret;
    //    }
    int index = p_frame->type;
    FrameHandler handler = usbFrameHandlers[index];
    if (handler != NULL)
    {
        handler(p_frame);
    }
    else
    {
        NRF_LOG_WARNING("Recved USB Frame Type: %d, but no handler", p_frame->type);
    }

    return ERROR_NO_ERROR;
}


void  receiveStreamFrame(Frame * p_frame)
{

    int index = p_frame->type;
    FrameHandler handler = usbFrameHandlers[index];
    if (handler != NULL)
    {
        handler(p_frame);
    }
    else
    {
        NRF_LOG_WARNING("Recved stream USB Frame Type: %d, but no handler", p_frame->type);
    }

}

ProtocolErrorCode sendUSBFrame(Frame *p_frame)
{
    uint8_t data[WIRELESS_BUF_SIZE];
    uint16_t data_len = 0;
    p_frame->seq = getSendSeq(&mSendSeq);
    frameToData(p_frame, data, &data_len);

    // Send data through CDC ACM
    int32_t ret = app_usbd_cdc_acm_write(getUSBCdcAcm(), data, data_len);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Send USB Frame ERROR: %d", ret);
        return ERROR_SEND_FAILED;
    }
    return ERROR_NO_ERROR;
}

ProtocolErrorCode sendUSBLongFrame(Frame *p_frame, uint8_t *p_data)
{

    if (p_frame == NULL || p_data == NULL)
    {
        return ERROR_INVALID_PARAM;
    }
    p_frame->seq = getSendSeq(&mSendSeq);

    uint16_t data_len = FRAME_HEADER_LEN + p_frame->dataLen + 1;
    uint8_t *sendData;
    sendData = malloc(data_len);

    memcpy(sendData, p_frame, FRAME_HEADER_LEN);
    memcpy(sendData + FRAME_HEADER_LEN, p_data, p_frame->dataLen);
    sendData[p_frame->dataLen + FRAME_HEADER_LEN] = 0x00;

    // Send data through CDC ACM
    int32_t ret = app_usbd_cdc_acm_write(getUSBCdcAcm(), sendData, data_len);

    free(sendData);
    sendData = NULL;

    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Send USB Frame ERROR: %d", ret);
        return ERROR_SEND_FAILED;
    }
    return ERROR_NO_ERROR;
}

#include "G3_Protocol.h"
#include "G3_ProtocolInterface.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "extern_resources.h"
#include "IcuState.h"
#include "G3_seq.h"
#include "FpgaInterface.h"
#include <stdlib.h>
#include "nrf_delay.h"
#include "bsp_btn_ble.h"
#include "nrf_drv_ppi.h"
#include "ble_nus.h"
#include "nrf_drv_saadc.h"
#include <math.h>
int commondAddr = 0;
nrf_saadc_value_t  saadc_val;
float  val;  //实际测量值
float   rNtc;//电阻值
APP_TIMER_DEF(m_heart_beat_timer);
/**
 * @brief 全局统一的发送方帧序号
 *
 */
static uint16_t mFrmSendSeq = 0;

/**
 * @brief 全局统一的BLE接收帧序号
 *
 */
static uint16_t mCurrentBleRcvSeq = 0;
int frmindex = 0;

uint16_t getBleSendSeq(void)
{
    return getSendSeq(&mFrmSendSeq);
}

static FrameHandler bleFrameHandlers[FRAME_HANDLER_NUM] = {NULL};
uint8_t commndList[WIRELESS_BUF_SIZE] = {0};
uint8_t bufferData[WIRELESS_BUF_SIZE] = {0};
void initProtocol(void)
{
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
    bleFrameHandlers[FRAME_SAMPLING_CONTROL] = frmHandlerSamplingControl;
	  bleFrameHandlers[FRAME_CHANGE_MODE] = frmHandlerChangeMode;
	  bleFrameHandlers[FRAME_GET_TEMPERATURE] = frmHandlerGetTemperature;
	  bleFrameHandlers[FRAME_GET_RSSI] = frmHandlerGetRssi;
	  bleFrameHandlers[FRAME_GET_POWER] = frmHandlerGetPower;
}
static void frmHandlerChangeMode(Frame *frame)
{
    NRF_LOG_INFO("Recv frmHandlerChangeMode Frame from central:");
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmUpdateTriggerOuts = (Frame *)data;
    frmUpdateTriggerOuts->header = HEADER;
    frmUpdateTriggerOuts->seq = getBleSendSeq();
    frmUpdateTriggerOuts->type = frame->type;
    frmUpdateTriggerOuts->dataLen = 0x01;
    frmUpdateTriggerOuts->data[0] = 0x00;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set IsTriggered report TO ECU FAILED!");
    }
		if(frame->data[0])
		{
		  isTestMode=true;
			NRF_LOG_INFO("set testmode:");
		}	
		else
		{	 
			isTestMode=false;	 
		}
		
}
static void frmHandlerGetRssi(Frame *frame)
{
    NRF_LOG_INFO("Recv frmHandlerGetRssi Frame from central:");
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
	  int8_t rssi;
    uint8_t connectChannel;
    sd_ble_gap_rssi_get(m_conn_handle, &rssi,&connectChannel);
	  NRF_LOG_INFO("rssi is %d!",rssi);
    Frame *frmUpdateTriggerOuts = (Frame *)data;
    frmUpdateTriggerOuts->header = HEADER;
    frmUpdateTriggerOuts->seq = getBleSendSeq();
    frmUpdateTriggerOuts->type = frame->type;
    frmUpdateTriggerOuts->dataLen = 0x01;
    frmUpdateTriggerOuts->data[0] =(uint8_t)rssi;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set IsTriggered report TO ECU FAILED!");
    }
}
static void frmHandlerGetPower(Frame *frame)
{
 
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
		uint8_t power[2]= {0};
    read_time_to_empty(power);
    Frame *frmUpdateTriggerOuts = (Frame *)data;
    frmUpdateTriggerOuts->header = HEADER;
    frmUpdateTriggerOuts->seq = getBleSendSeq();
    frmUpdateTriggerOuts->type = frame->type;
    frmUpdateTriggerOuts->dataLen = 0x02;
		NRF_LOG_INFO("Recv frmHandlerGetPower Frame from central:");
    frmUpdateTriggerOuts->data[0] = power[0];
		frmUpdateTriggerOuts->data[1] = power[1];
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set frmHandlerGetPower report TO ECU FAILED!");
    }
}
static void frmHandlerGetTemperature(Frame *frame)
{
    NRF_LOG_DEBUG("Recv UpdateTriggerOutsBLE Frame from central:");
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmUpdateTriggerOuts = (Frame *)data;
    frmUpdateTriggerOuts->header = HEADER;
    frmUpdateTriggerOuts->seq = getBleSendSeq();
    frmUpdateTriggerOuts->type = frame->type;
    frmUpdateTriggerOuts->dataLen = 0x01;
    frmUpdateTriggerOuts->data[0] = 0x20;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set IsTriggered report TO ECU FAILED!");
    }
}

static void frmHandlerSamplingControl(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv Handler Sampling Control BLE Frame from central:");
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *SamplingControl = (Frame *)data;
    SamplingControl->header = HEADER;
    SamplingControl->seq = getBleSendSeq();
    SamplingControl->type = p_frame->type;
    SamplingControl->dataLen = 1;
    SamplingControl->data[0] = 0;
    NRF_LOG_DEBUG("data0: 0x%08X", p_frame->data[0]);
    if (p_frame->data[0] == 0)
    {
        // 将 GPIO 引脚设置为低电平
				  nrf_gpio_pin_clear(GPIO_FPGA_PIN);
			    nrf_delay_ms(100);
          nrf_gpio_pin_clear(GPIO_STOP_PIN);
				  nrf_gpio_pin_clear(GPIO_30W_SMPIN);
		
    }
    else
    {
	
        nrf_gpio_pin_set(GPIO_STOP_PIN);
			  nrf_delay_ms(100);
				nrf_gpio_pin_set(GPIO_FPGA_PIN);
		
    }

    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), SamplingControl);

    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Handler Sampling Control report TO ECU FAILED!");
    }
    else
    {
        NRF_LOG_DEBUG("Handler Sampling Control report TO ECU success!");
    }
}
static void frmHandlerUploadCommond(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv Handler Upload Commond BLE Frame from central:");
    NRF_LOG_DEBUG("p_frame len is %d", p_frame->dataLen);
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);
    uint32_t err_code;
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *HandlerUploadCommond = (Frame *)data;
    HandlerUploadCommond->header = HEADER;
    HandlerUploadCommond->seq = getBleSendSeq();
    HandlerUploadCommond->type = p_frame->type;
    HandlerUploadCommond->dataLen = 0x01;
    if (p_frame->data[0] == 0x00)
    {
        commondAddr = p_frame->data[1];
        uint32_t adress;
        if (commondAddr == 0)
        {
            adress = 0x000000C0;
        }
        else if (commondAddr == 1)
        {
            adress = 0x000000C1;
        }
        else
        {
            adress = 0x000000C2;
        }
        NRF_LOG_DEBUG("adress: 0x%08X", adress);
        frmindex += p_frame->dataLen - 2;
        memcpy(commndList, p_frame->data + 2, p_frame->dataLen - 2);
        err_code = write_data(adress, commndList, frmindex);
        HandlerUploadCommond->data[0] = err_code;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), HandlerUploadCommond);
        frmindex = 0;
        memset(commndList, 0, WIRELESS_BUF_SIZE);
        if (ret != ERROR_NO_ERROR)
        {
            NRF_LOG_DEBUG("Upload Commond report TO ECU FAILED!");
        }
        else
        {
            NRF_LOG_DEBUG("Upload Commond report TO ECU success!");
        }
    }

    else if (p_frame->data[0] == 0x01 || p_frame->data[0] == 0x02)
    {

        if (p_frame->data[0] == 0x01)
        {
            commondAddr = p_frame->data[1];
            memcpy(commndList + frmindex, p_frame->data + 2, p_frame->dataLen - 2);
            frmindex += p_frame->dataLen - 2;
            NRF_LOG_DEBUG("index %d", frmindex);
        }
        else
        {
            frmindex += p_frame->dataLen - 1;
            NRF_LOG_DEBUG("index %d", frmindex);
            memcpy(commndList + frmindex, p_frame->data + 1, p_frame->dataLen - 1);
        }
        HandlerUploadCommond->data[0] = 0x00;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), HandlerUploadCommond);
    }
    else
    {
        frmindex += p_frame->dataLen - 1;
        memcpy(commndList + frmindex, p_frame->data + 1, p_frame->dataLen - 1);
        uint32_t adress;
        if (commondAddr == 0x00)
        {
            adress = 0x000000C0;
        }
        else if (commondAddr == 1)
        {
            adress = 0x000000C1;
        }
        else
        {
            adress = 0x000000C2;
        }
        NRF_LOG_DEBUG("index %d", frmindex);
        err_code = write_data(adress, commndList, frmindex);
        HandlerUploadCommond->data[0] = err_code;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), HandlerUploadCommond);
        frmindex = 0;
        memset(commndList, 0, WIRELESS_BUF_SIZE);
    }
}

static void frmHandlerSetWireInValue(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv Set Wire In Value BLE Frame from central:");
    NRF_LOG_DEBUG("p_frame len is %d", p_frame->dataLen);
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *SetWireInValue = (Frame *)data;
    SetWireInValue->header = HEADER;
    SetWireInValue->seq = getBleSendSeq();
    SetWireInValue->type = p_frame->type;
    SetWireInValue->dataLen = 0x01;
    uint8_t rwAddr[4], rwValue[4], mask[4];
    splitArray(p_frame->data, rwAddr, rwValue, mask);
    int data_len = 4;
    uint32_t err_code;
    uint8_t buffer_rx[4] = {0};
    uint32_t adress = convertToUint32(rwAddr);
    NRF_LOG_DEBUG("adress: 0x%08X", adress);
		int index=(int)adress*4;
		memcpy(buffer_rx,bufferData+index,4);
		NRF_LOG_DEBUG("adress  to int : %d", index);
    NRF_LOG_DEBUG("  buffer_rx data is 0x%08X\n", *(unsigned int *)(buffer_rx));
    uint8_t little_endian[data_len];
    for (size_t i = 0; i < data_len; i++)
    {
        little_endian[i] = buffer_rx[data_len - i - 1];
    }

    setRegisterValue(little_endian, rwValue, mask);
    NRF_LOG_DEBUG("  littel SetWireInValue data is 0x%08X\n", *(unsigned int *)(little_endian));

    uint8_t temp_buffer[data_len];

    for (size_t i = 0; i < data_len; i++)
    {
        temp_buffer[i] = little_endian[data_len - i - 1];
    }

    NRF_LOG_DEBUG(" BIG SetWireInValue data is 0x%08X\n", *(unsigned int *)(temp_buffer));	
		memcpy(bufferData+index,temp_buffer,4);
    err_code = write_data(adress, temp_buffer, data_len);
    SetWireInValue->data[0] = err_code;

    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), SetWireInValue);
    if (err_code != ERROR_NO_ERROR)
    {
        if (err_code == NRF_ERROR_RESOURCES)
        {
            nrf_delay_ms(100);
            err_code = sendFrame(getBLE_NUS(), SetWireInValue);
            if (err_code == ERROR_NO_ERROR)
            {
                NRF_LOG_DEBUG("Set SetWireInValue report TO ECU success!");
            }
            else
            {
                NRF_LOG_DEBUG("Set SetWireInValue report TO ECU FAILED!");
            }
        }
        else
        {
            NRF_LOG_DEBUG("Set SetWireInValue report TO ECU FAILED!");
        }
    }
    else
    {
        NRF_LOG_DEBUG("Set SetWireInValue report TO ECU success!");
    }
}

static void frmHandlerUpdateWireIn(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv UpdateWireIn BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);

    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmIcustatus = (Frame *)data;
    frmIcustatus->header = HEADER;
    frmIcustatus->seq = getBleSendSeq();
    frmIcustatus->type = p_frame->type;
    frmIcustatus->dataLen = 0x01;
    frmIcustatus->data[0] = 0x00;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmIcustatus);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set UpdateWireIn report TO ECU FAILED!");
    }
}

static void frmHandlerGetWireInValue(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv Get Wire In Value BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmWireInValue = (Frame *)data;
    frmWireInValue->header = HEADER;
    frmWireInValue->seq = getBleSendSeq();
    frmWireInValue->type = p_frame->type;
    frmWireInValue->dataLen = 0x01;
    NRF_LOG_DEBUG("read buffer");
    frmWireInValue->data[0] = 0x00;
    NRF_LOG_DEBUG("send buffer");
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmWireInValue);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set Wire In Value report TO ECU FAILED!");
    }
}

static void frmHandlerUpdateWireOut(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv UpdateWireOut BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);

    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmIcustatus = (Frame *)data;
    frmIcustatus->header = HEADER;
    frmIcustatus->seq = getBleSendSeq();
    frmIcustatus->type = p_frame->type;
    frmIcustatus->dataLen = 0x01;
    frmIcustatus->data[0] = 0x00;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmIcustatus);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set UpdateWireOut report TO ECU FAILED!");
    }
}

static void frmHandlerGetWireOutValue(Frame *p_frame)
{
    NRF_LOG_DEBUG("Recv GetWireOutValue BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(p_frame, FRAME_HEADER_LEN + p_frame->dataLen + 1);

    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *getWireOutValue = (Frame *)data;
    getWireOutValue->header = HEADER;
    getWireOutValue->seq = getBleSendSeq();
    getWireOutValue->type = p_frame->type;
    getWireOutValue->dataLen = 0x04;
    int data_len = 4;
    uint32_t err_code;
    uint8_t buffer_rx[QSPI_PAGE_SIZE] = {0};
    uint32_t adress = convertToUint32(p_frame->data);
    NRF_LOG_DEBUG("adress: 0x%08X", adress);
    err_code = read_data(adress, buffer_rx, data_len);
    if (err_code != NRFX_SUCCESS)
    {
        NRF_LOG_DEBUG(" read_data  fail");
        getWireOutValue->data[0] = 0x00;
        sendFrame(getBLE_NUS(), getWireOutValue);
    }
    NRF_LOG_DEBUG(" getWireOutValue data is 0x%08X\n", *(unsigned int *)(buffer_rx));

    uint8_t little_endian[data_len];
    for (size_t i = 0; i < data_len; i++)
    {
        little_endian[i] = buffer_rx[data_len - i - 1];
    }
    uint8_t *content = (uint8_t *)(data + FRAME_HEADER_LEN);
    memcpy(content, little_endian, data_len);
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), getWireOutValue);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set GetWireOutValue report TO ECU FAILED!");
    }
}

static void frmHandlerICUStatus(Frame *frame)
{
    NRF_LOG_DEBUG("Recv ICU Status BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);
    initICUState();
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmIcustatus = (Frame *)data;
    frmIcustatus->header = HEADER;
    frmIcustatus->seq = getBleSendSeq();
    frmIcustatus->type = FRAME_ICU_STATUS;
    frmIcustatus->dataLen = 0x01;
    frmIcustatus->data[0] = getIcuStatus();
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmIcustatus);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("ICU Status report TO ECU FAILED!");
    }
}

static void frmHandlerActivateTriggerIn(Frame *frame)
{
    NRF_LOG_DEBUG("Recv ActivateTriggerIn BLE Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmActivateTriggerIn = (Frame *)data;
    frmActivateTriggerIn->header = HEADER;
    frmActivateTriggerIn->seq = getBleSendSeq();
    frmActivateTriggerIn->type = frame->type;
    frmActivateTriggerIn->dataLen = 0x01;
    if (frame->dataLen != 5)
    {
        frmActivateTriggerIn->data[0] = 0x01;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmActivateTriggerIn);
        return;
    }
    uint8_t rwAddr[4];
    rwAddr[0] = frame->data[0];
    rwAddr[1] = frame->data[1];
    rwAddr[2] = frame->data[2];
    rwAddr[3] = frame->data[3];
    int indexBit = frame->data[4];
    int data_len = 4;
    uint32_t err_code;
    uint8_t buffer_rx[QSPI_PAGE_SIZE] = {0};
    uint32_t adress = convertToUint32(rwAddr);
		int index=(int)adress*4;
		memcpy(buffer_rx,bufferData+index,4);
		NRF_LOG_DEBUG("adress  to int : %d", index);
    //err_code = read_data(adress, buffer_rx, data_len);
    NRF_LOG_DEBUG(" buffer_rx data is 0x%08X\n", *(unsigned int *)(buffer_rx));
    uint8_t little_endian[data_len];
    for (size_t i = 0; i < data_len; i++)
    {
        little_endian[i] = buffer_rx[data_len - i - 1];
    }
    *little_endian |= (1u << indexBit);
    uint8_t temp_buffer[data_len];
    for (size_t i = 0; i < data_len; i++)
    {
        temp_buffer[i] = little_endian[data_len - i - 1];
    }

    NRF_LOG_DEBUG(" ActivateTriggerIn data is 0x%08X\n", *(unsigned int *)(temp_buffer));		
	  memcpy(bufferData+index,temp_buffer,4);
    err_code = write_data(adress, temp_buffer, data_len);
    if (err_code != ERROR_NO_ERROR)
    {
        frmActivateTriggerIn->data[0] = err_code;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmActivateTriggerIn);
    }

    else
    {
        frmActivateTriggerIn->data[0] = 0x00;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmActivateTriggerIn);
        if (err_code != ERROR_NO_ERROR)
        {
            if (err_code == NRF_ERROR_RESOURCES)
            {
                nrf_delay_ms(100);
                err_code = sendFrame(getBLE_NUS(), frmActivateTriggerIn);
                if (err_code == ERROR_NO_ERROR)
                {
                    NRF_LOG_DEBUG("Set frmActivateTriggerIn report TO ECU success!");
                }
            }
            else
            {

                NRF_LOG_DEBUG("Set frmActivateTriggerIn report TO ECU FAILED!");
            }
        }
        else
        {
            NRF_LOG_DEBUG("Set frmActivateTriggerIn report TO ECU success!");
        }
    }
}

static void frmHandlerUpdateTriggerOuts(Frame *frame)
{
    NRF_LOG_DEBUG("Recv UpdateTriggerOutsBLE Frame from central:");
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmUpdateTriggerOuts = (Frame *)data;
    frmUpdateTriggerOuts->header = HEADER;
    frmUpdateTriggerOuts->seq = getBleSendSeq();
    frmUpdateTriggerOuts->type = frame->type;
    frmUpdateTriggerOuts->dataLen = 0x01;
    frmUpdateTriggerOuts->data[0] = 0x00;
    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_DEBUG("Set IsTriggered report TO ECU FAILED!");
    }
}

static void frmHandlerIsTriggered(Frame *frame)
{
    NRF_LOG_DEBUG("Recv IsTriggered Frame from central:");
    NRF_LOG_HEXDUMP_DEBUG(frame, FRAME_HEADER_LEN + frame->dataLen + 1);
    uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
    Frame *frmIsTriggered = (Frame *)data;
    frmIsTriggered->header = HEADER;
    frmIsTriggered->seq = getBleSendSeq();
    frmIsTriggered->type = frame->type;
    frmIsTriggered->dataLen = 0x01;
    if (frame->dataLen != 5)
    {
        NRF_LOG_DEBUG("dataLen : %d", frame->dataLen);
        frmIsTriggered->data[0] = 0x01;
        ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmIsTriggered);
        return;
    }
    uint8_t rwAddr[4];
    rwAddr[0] = frame->data[0];
    rwAddr[1] = frame->data[1];
    rwAddr[2] = frame->data[2];
    rwAddr[3] = frame->data[3];
    int indexBit = frame->data[4];
    int data_len = 4;
    NRF_LOG_DEBUG("index: %d", indexBit);
    uint8_t buffer_rx[QSPI_PAGE_SIZE] = {0};
    uint32_t adress = convertToUint32(rwAddr);
    NRF_LOG_DEBUG("adress: 0x%08X", adress);
    read_data(adress, buffer_rx, data_len);
    NRF_LOG_DEBUG("  is Triggered  buffer_rx data is 0x%08X\n", *(unsigned int *)(buffer_rx));

    uint8_t little_endian[data_len];
    for (size_t i = 0; i < data_len; i++)
    {
        little_endian[i] = buffer_rx[data_len - i - 1];
    }
    NRF_LOG_DEBUG("  littel ActivateTriggerIn data is 0x%08X\n", *(unsigned int *)(little_endian));

    if (*(uint32_t *)(little_endian) & (1u << indexBit))
    {
        frmIsTriggered->data[0] = 0x01;
    }
    else
    {
        frmIsTriggered->data[0] = 0x00;
    }

    ret_code_t err_code = sendFrame(getBLE_NUS(), frmIsTriggered);
    if (err_code != ERROR_NO_ERROR)
    {
        if (err_code == NRF_ERROR_RESOURCES)
        {
            nrf_delay_ms(100);
            err_code = sendFrame(getBLE_NUS(), frmIsTriggered);
            if (err_code == ERROR_NO_ERROR)
            {
                NRF_LOG_DEBUG("Set IsTriggered report TO ECU success!");
            }
        }
        else
        {
            NRF_LOG_DEBUG("Set IsTriggered report TO ECU FAILED!");
        }
    }
    else
    {

        NRF_LOG_DEBUG("Set IsTriggered report TO ECU success!");
    }
}

ProtocolErrorCode receiveFrame(uint8_t *p_data, uint16_t data_len)
{
    if (p_data == NULL || data_len == 0)
    {
        return ERROR_INVALID_PARAM;
    }

    Frame *frame = (Frame *)p_data;
    ProtocolErrorCode ret = dataToFrame(p_data, data_len, frame);

    if (ret != ERROR_NO_ERROR)
    {
        NRF_LOG_WARNING("Data to Frame ERROR: %d", ret);
        return ret;
    }
    mCurrentBleRcvSeq = frame->seq;

    if (bleFrameHandlers[frame->type] != NULL)
    {
        bleFrameHandlers[frame->type](frame);
    }
    else
    {
        NRF_LOG_WARNING("Recved BLE Frame Type: %d, but no handler", frame->type);
    }
    return ERROR_NO_ERROR;
}

void heartBeatTimeoutHandler(void *p_context)
{

	  NRF_WDT->RR[0]=0x6E524635UL;
//    Frame heatbeatFrm;
//    memset(&heatbeatFrm, 0, sizeof(Frame));
//    heatbeatFrm.header = HEADER;
//    heatbeatFrm.seq = 0x0000;
//    heatbeatFrm.type = FRAME_HEARTBEAT;
//    heatbeatFrm.dataLen = 0x0000;
//    sendFrame(p_context, &heatbeatFrm);
//	  uint8_t data[MAX_BLE_FRAME_SIZE] = {0};
//	  int8_t rssi;
//    uint8_t connectChannel;
//    sd_ble_gap_rssi_get(m_conn_handle, &rssi,&connectChannel);
//    Frame *frmUpdateTriggerOuts = (Frame *)data;
//    frmUpdateTriggerOuts->header = HEADER;
//    frmUpdateTriggerOuts->seq = getBleSendSeq();
//    frmUpdateTriggerOuts->type = FRAME_GET_RSSI;
//    frmUpdateTriggerOuts->dataLen = 0x01;
//    frmUpdateTriggerOuts->data[0] =(uint8_t)rssi;
//    ProtocolErrorCode ret = sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
//   
//    frmUpdateTriggerOuts->header = HEADER;
//    frmUpdateTriggerOuts->seq = getBleSendSeq();
//    frmUpdateTriggerOuts->type = FRAME_GET_TEMPERATURE;
//    frmUpdateTriggerOuts->dataLen = 0x01;
//    frmUpdateTriggerOuts->data[0] = 0x20;
//    sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
//		
//		NRF_LOG_DEBUG("Recv frmHandlerGetPower Frame from central:");
// 
//    
//    frmUpdateTriggerOuts->header = HEADER;
//    frmUpdateTriggerOuts->seq = getBleSendSeq();
//    frmUpdateTriggerOuts->type =FRAME_GET_POWER;
//    frmUpdateTriggerOuts->dataLen = 0x02;
//    frmUpdateTriggerOuts->data[0] = 0xff;
//		frmUpdateTriggerOuts->data[1] = 0xff;
//    sendFrame(getBLE_NUS(), frmUpdateTriggerOuts);
 

}

ProtocolErrorCode startHeartBeats(void *bleService)
{
    ret_code_t err_code = app_timer_create(&m_heart_beat_timer, APP_TIMER_MODE_REPEATED, heartBeatTimeoutHandler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_heart_beat_timer, APP_TIMER_TICKS(HEARTBEAT_INTERVAL), bleService);
    APP_ERROR_CHECK(err_code);
    //*配置看门狗*//
	  //配置看门狗重载值
    NRF_WDT->CRV=65536*10;
	  //配置看门狗休眠下运行
  	NRF_WDT->CONFIG=0x01;
	  //申请喂狗通道，也就是使用哪个RR
  	NRF_WDT->RREN=0x01;	
	 //启动WDT
    NRF_WDT->TASKS_START=1;
		 
	   //*配置看门狗中断*//
		//使能看门狗定时器超时事件
	  NRF_WDT->EVENTS_TIMEOUT=1; 
		//使能看门狗中断
    NRF_WDT->INTENSET=1;
    //使能看门中断嵌套
	  NVIC_EnableIRQ(WDT_IRQn);
    NRF_LOG_DEBUG("Heartbeat timer created.");
    return ERROR_NO_ERROR;
}

ProtocolErrorCode stopHeartBeats(void)
{
    ret_code_t err_code = app_timer_stop(m_heart_beat_timer);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEBUG("Heartbeat timer stopped.");
    return ERROR_NO_ERROR;
}

ProtocolErrorCode sendFrame(void *bleService, Frame *p_frame)
{
    uint8_t data[WIRELESS_BUF_SIZE];
    uint16_t data_len = 0;
    frameToData(p_frame, data, &data_len);
    ble_nus_t *m_nus = (ble_nus_t *)bleService;
    int32_t ret = ble_nus_data_send(m_nus, data, &data_len, m_conn_handle);
    if (ret != NRF_SUCCESS && ret != NRF_ERROR_RESOURCES && ret != NRF_ERROR_NOT_FOUND)
    {
        NRF_LOG_WARNING("Send Frame ERROR: %d", ret);
        return ret;
    }
    return ret;
}
ProtocolErrorCode sendBLELongFrame(void *bleService, Frame *p_frame, uint8_t *p_data)
{

    if (p_frame == NULL || p_data == NULL)
    {
        return ERROR_INVALID_PARAM;
    }
    p_frame->seq = getBleSendSeq();

    uint16_t data_len = FRAME_HEADER_LEN + p_frame->dataLen + 1;
    uint8_t *sendData;
    sendData = (uint8_t *)malloc(data_len);
    memcpy(sendData, p_frame, FRAME_HEADER_LEN);
    memcpy(sendData + FRAME_HEADER_LEN, p_data, p_frame->dataLen);
    sendData[p_frame->dataLen + FRAME_HEADER_LEN] = 0x00;
    ble_nus_t *m_nus = (ble_nus_t *)bleService;
    int32_t ret = ble_nus_data_send(m_nus, sendData, &data_len, m_conn_handle);
    free(sendData);
    sendData = NULL;

    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_WARNING("Send BLE Frame ERROR: %d", ret);
        return ERROR_SEND_FAILED;
    }
    return ERROR_NO_ERROR;
}
void splitArray(const uint8_t array[12], uint8_t first[4], uint8_t middle[4], uint8_t last[4])
{
    for (int i = 0; i < 12; ++i)
    {
        if (i < 4)
        {
            first[i] = array[i];
        }
        else if (i < 8)
        {
            middle[i - 4] = array[i];
        }
        else
        {
            last[i - 8] = array[i];
        }
    }
}
void setRegisterValue(uint8_t registerValue[4], const uint8_t value[4], const uint8_t mask[4])
{
    for (int i = 0; i < 4; ++i)
    {
        registerValue[i] = (registerValue[i] & ~mask[i]) | (value[i] & mask[i]);
    }
}
uint32_t convertToUint32(uint8_t array[4])
{
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i)
    {
        result |= ((uint32_t)array[i] << (8 * i));
    }
    return result;
}
void syntheticDataFrame(uint8_t *data)
{
    static uint32_t timestamp = 0;
    uint16_t *pWrite = (uint16_t *)data;
    uint64_t header = 0xd7a22aaa38132a53UL;
    pWrite[0] = (uint16_t)((header & 0x000000000000ffffUL) >> 0);
    pWrite[1] = (uint16_t)((header & 0x00000000ffff0000UL) >> 16);
    pWrite[2] = (uint16_t)((header & 0x0000ffff00000000UL) >> 32);
    pWrite[3] = (uint16_t)((header & 0xffff000000000000UL) >> 48);
    pWrite += 4;
    // Write timestamp.
    pWrite[0] = (uint16_t)((timestamp & 0x0000ffffU) >> 0);
    pWrite[1] = (uint16_t)((timestamp & 0xffff0000U) >> 16);
    pWrite += 2;
    for (int channel = 0; channel < 3; ++channel)
    {
        *pWrite = 0U;
        pWrite++;
    }

    for (int channel = 0; channel < 32; ++channel)
    {
        double freqHz = 10;
        double tStepMsec = 1.0e3 / 500;
        double periodMsec = 1000.0 / freqHz;
        double index = timestamp % 500;
        double result = 100;//100 * sin(3.1415926 * 2 * tStepMsec * index / periodMsec); // 150.0    ;  //* sin(3.1415926*2 * tStepMsec * index / periodMsec);
        const uint16_t value = result / 0.195 + 32768;
        for (int stream = 0; stream < 2; ++stream)
        {
            *pWrite = value;
            pWrite++;
        }
    }
    // Write Digital In data.
    *pWrite = (uint16_t)(32768U);
    pWrite++;

    // Write Digital Out data.
    *pWrite = (uint16_t)(32768U);
    timestamp++;
}
void testGpiote()
{

    NRF_LOG_DEBUG("testgpio");
    uint8_t buffer_tx[4];
    uint32_t rwAddr02 = 0x00000002;
    buffer_tx[0] = 0x00;
    buffer_tx[1] = 0x00;
    buffer_tx[2] = 0x01;
    buffer_tx[3] = 0xF4;
    write_data(rwAddr02, buffer_tx, 4);
    nrf_delay_ms(100);

    uint32_t rwAddr40 = 0x00000040;
    buffer_tx[0] = 0x00;
    buffer_tx[1] = 0x00;
    buffer_tx[2] = 0x00;
    buffer_tx[3] = 0x00;
    write_data(rwAddr40, buffer_tx, 4);
    nrf_delay_ms(100);

    buffer_tx[0] = 0x00;
    buffer_tx[1] = 0x00;
    buffer_tx[2] = 0x00;
    buffer_tx[3] = 0x01;
    write_data(rwAddr40, buffer_tx, 4);
    nrf_delay_ms(100);

    uint32_t rwAddr01 = 0x00000001;
    buffer_tx[0] = 0xff;
    buffer_tx[1] = 0xff;
    buffer_tx[2] = 0xff;
    buffer_tx[3] = 0xff;
    write_data(rwAddr01, buffer_tx, 4);
    nrf_delay_ms(100);

    uint32_t rwAddr21 = 0x00000021;
    buffer_tx[0] = 0x00;
    buffer_tx[1] = 0x00;
    buffer_tx[2] = 0x00;
    buffer_tx[3] = 0x00;
    write_data(rwAddr21, buffer_tx, 4);
    nrf_delay_ms(100);

    buffer_tx[0] = 0x00;
    buffer_tx[1] = 0x00;
    buffer_tx[2] = 0x00;
    buffer_tx[3] = 0x01;
    write_data(rwAddr21, buffer_tx, 4);
    nrf_delay_ms(100);
}

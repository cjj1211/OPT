/******************************************************************************
*  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
*  作者 : 宋廷宇 tingyu.song@neuroxess.com
*  描述 : 定义了USB通信协议的接口
*  修改记录: 
*  
******************************************************************************/
#ifndef __G3_PROTOCOLUSB_H__
#define __G3_PROTOCOLUSB_H__

#include "G3_Protocol.h"

extern uint16_t currentRcvUSBSeq;


/**
 * @brief 初始化USB接口协议
 * 
 */
void initUSBProtocol(void);


/**
 * @brief 重置发送帧序号
 * 
 */
void resetUSBSendSeq(void);

/**
 * @brief 重置接收帧序号
 * 
 */
void resetUSBRcvSeq(void);

/**
 * @brief 对USB接口接收到的数据进行帧解析
 * 
 * @param p_data 
 * @param data_len 
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode receiveUSBFrame(uint8_t * p_data, uint16_t data_len);

/**
 * @brief 通过USB接口发送帧
 * 
 * @param p_frame 
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode sendUSBFrame(Frame * p_frame);

/**
 * @brief  对于数据字节大于1的数据进行数据发送帧
 * 
 * @param p_frame  data
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode sendUSBLongFrame(Frame * p_frame,uint8_t* data);

#endif // !__G3_PROTOCOLUSB_H__


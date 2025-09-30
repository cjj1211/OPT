/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。                   
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  描述 : 定义了urt通信协议的接口
*  修改记录: 
*  
******************************************************************************/
#ifndef __G3_PROTOCOLUart_H__
#define __G3_PROTOCOLUart_H__

#include "G3_Protocol.h"

extern uint16_t currentRcvUartSeq;


/**
 * @brief 初始化Uart接口协议
 * 
 */
void initUartProtocol(void);


/**
 * @brief 重置发送帧序号
 * 
 */
void resetUartSendSeq(void);

/**
 * @brief 重置接收帧序号
 * 
 */
void resetUartRcvSeq(void);

/**
 * @brief 对Uart接口接收到的数据进行帧解析
 * 
 * @param p_data 
 * @param data_len 
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode receiveUartFrame(uint8_t * p_data, uint16_t data_len);

/**
 * @brief 通过Uart接口发送帧
 * 
 * @param p_frame 
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode sendUartFrame(Frame * p_frame);

#endif // !__G3_PROTOCOLUart_H__


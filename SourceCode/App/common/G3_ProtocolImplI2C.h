/******************************************************************************
*  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。                   
*  作者 : 崔俊杰 junjie.cui@neuroxess.com
*  描述 : 定义了I2C通信协议的接口
*  修改记录: 
*  
******************************************************************************/
#ifndef __G3_PROTOCOLI2C_H__
#define __G3_PROTOCOLI2C_H__

#include "G3_Protocol.h"
#include <stddef.h>
#include <stdbool.h>
extern uint16_t currentRcvI2CSeq;


/**
 * @brief 初始化I2C接口协议
 * 
 */
void initI2CProtocol(void);


/**
 * @brief 重置发送帧序号
 * 
 */
void resetI2CSendSeq(void);

/**
 * @brief 重置接收帧序号
 * 
 */
void resetI2CRcvSeq(void);

/**
 * @brief 对I2C接口接收到的数据进行帧解析
 * 
 * @param p_data 
 * @param data_len 
 * @return ProtocolErrorCode 
 */
ProtocolErrorCode receiveI2CFrame(uint8_t * p_data, uint16_t data_len);

/**
 * @brief 通过I2C接口发送帧
 * 
 * @param p_frame   
  * @param  slaveAddress   设备地址
  * @param reg_address     寄存器地址
 * @return ProtocolErrorCode 
 */
 ProtocolErrorCode writeFrame(uint8_t slaveAddress, uint8_t  reg_address,Frame *p_frame) ;//发送数据

/**
 * @brief 通过I2C接口读取帧
 * 
 * @param p_frame   
  * @param  slaveAddress   设备地址
  * @param reg_address     寄存器地址
 * @return ProtocolErrorCode 
 */
 ProtocolErrorCode readFrame(uint8_t slaveAddress, uint8_t  reg_address,Frame *p_frame) ;//读取数据

#endif // !__G3_PROTOCOLI2C_H__


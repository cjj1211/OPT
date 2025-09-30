/******************************************************************************
 *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  描述 : 定义了G3无线数据传输协议的接口函数
 *  修改记录:
 *
 ******************************************************************************/

//#include "ble_nus.h"
#include "G3_Protocol.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// static volatile bool gpioFrame = false;
/**
 * @brief 获取BLE发送序列号
 *
 * @return uint16_t
 */
uint16_t getBleSendSeq(void);

/**
 * @brief 重置BLE发送序列号
 *
 */
void resetBleSendSeq(void);

/**
 * @brief 重置BLE接收序列号
 *
 */
void resetBleRcvSeq(void);

/**
 * @brief 对序列化数据进行帧解析
 *
 * @param p_data
 * @param data_len
 * @param p_frame
 * @return int 错误码
 */
//ProtocolErrorCode dataToFrame(uint8_t *p_data, uint16_t data_len, Frame *p_frame);

/**
 * @brief 将帧结构转换为序列化的数据
 *
 * @param p_frame
 * @param p_data
 * @param data_len
 * @return int
 */
ProtocolErrorCode frameToData(Frame *p_frame, uint8_t *p_data, uint16_t *data_len);

/**
 * @brief 对接收到的帧进行处理
 *
 * @param p_data
 * @param data_len
 */
//ProtocolErrorCode receiveFrame(uint8_t *p_data, uint16_t data_len);

/**
 * @brief 对接收到的流式帧进行处理
 *
 * @param p_data
 * @param data_len
 */
void receiveStreamFrame(Frame *p_frame);


ProtocolErrorCode startHeartBeats(void *bleService);

/**
 * @brief 停止心跳
 *
 * @return ProtocolErrorCode
 */
ProtocolErrorCode stopHeartBeats(void);



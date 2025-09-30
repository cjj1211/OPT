/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋廷宇 tingyu.song@neuroxess.com
 *  描述 : 定义了G3无线数据传输协议的接口函数
 *  修改记录:
 *
 ******************************************************************************/

#include "ble_nus.h"
#include "G3_Protocol.h"
#include <cstdint>
static bool isTestMode=false;
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
ProtocolErrorCode dataToFrame(uint8_t *p_data, uint16_t data_len, Frame *p_frame);

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
ProtocolErrorCode receiveFrame(uint8_t *p_data, uint16_t data_len);

/**
 * @brief 对接收到的流式帧进行处理
 *
 * @param p_data
 * @param data_len
 */
void receiveStreamFrame(Frame *p_frame);

/**
 * @brief 发送帧
 *
 * @param p_frame
 * @return int
 */
ProtocolErrorCode sendFrame(void *bleService, Frame *p_frame);

/**
 * @brief 启动心跳
 * @param bleService 用于发送数据的蓝牙服务，主机和外设不同，在实现时转换
 * @return ProtocolErrorCode
 */
ProtocolErrorCode startHeartBeats(void *bleService);

/**
 * @brief 停止心跳
 *
 * @return ProtocolErrorCode
 */
ProtocolErrorCode stopHeartBeats(void);

/**
 * @brief 协议处理初始化
 *
 */
void initProtocol(void);

/**
 * @brief 蓝牙连接是否已经建立
 *
 */
bool isBLEConnected(void);

/**
 * @brief 心跳帧处理器
 *
 * @param p_frame
 */
void frmHandlerHeartbeat(Frame *p_frame);

/**
 * @brief 获取ICU状态的协议帧处理器，ICU从机和主机的处理机制不同
 *
 * @param p_frame
 */
void frmHandlerICUStatus(Frame *p_frame);

/**
 * @brief 获取ICU信息的协议帧处理器，ICU从机和主机的处理机制不同
 *
 * @param p_frame
 */
void frmHandlerDeviceInfo(Frame *p_frame);

/**
 * @brief 获取已连接到上位机上的脑电设备列表
 *
 * @param p_frame
 */
void frmHandlerDeviceModelList(Frame *p_frame);

/**
 * @brief 	预修改配置参数
 *
 * @param p_frame
 */
void frmHandlerSetWireInValue(Frame *p_frame);

/**
 * @brief  使配置参数生效
 *
 * @param p_frame
 */
void frmHandlerUpdateWireIn(Frame *p_frame);

/**
 * @brief 获取配置参数当前值
 *
 * @param p_frame
 */
void frmHandlerGetWireInValue(Frame *p_frame);

/**
 * @brief 预读取WireOut端点值
 *
 * @param p_frame
 */
void frmHandlerUpdateWireOut(Frame *p_frame);

/**
 * @brief 读取WireOut端点值
 *
 * @param p_frame
 */
void frmHandlerGetWireOutValue(Frame *p_frame);

/**
 * @brief 激活触发器
 *
 * @param p_frame
 */
void frmHandlerActivateTriggerIn(Frame *p_frame);

/**
 * @brief 预读取触发器输出值
 *
 * @param p_frame
 */
void frmHandlerUpdateTriggerOuts(Frame *p_frame);

/**
 * @brief 触发器是否被激活
 *
 * @param p_frame
 */
/**
 * @brief 流式读取数据
 *
 * @param p_frame
 */

void frmHandlerReadBlockData(Frame *p_frame);

void frmHandlerIsTriggered(Frame *p_frame);

void frmHandlerSamplingControl(Frame *p_frame);
void frmHandlerChangeMode(Frame *p_frame);
void frmHandlerGetTemperature(Frame *p_frame);
void frmHandlerGetRssi(Frame *p_frame);
void frmHandlerGetPower(Frame *p_frame);

/**
 * @brief 上传指令
 *
 * @param p_frame
 */
void frmHandlerUploadCommond(Frame *p_frame);
/**
 * @brief 解析 写入地址,val,mask
 *
 * @param uint8_t array[12]   需要解析的数据
   @param uint8_t adress[4]   写入地址
   @param uint8_t val[4]  写入的数据
   @param uint8_t mask[4]  mask掩码值
 */

void splitArray(const uint8_t array[12], uint8_t adress[4], uint8_t val[4], uint8_t mask[4]);
/**
 * @brief 将原有数据通过掩码修改新写入数据
 * @param registerValue原始数据
 * @param value 新数据
   @param mask  掩码
 */
void setRegisterValue(uint8_t registerValue[4], const uint8_t value[4], const uint8_t mask[4]);
/**
 * @brief uint8_t转 uint32_t
 *
 * @param uint8_t
 */
uint32_t convertToUint32(uint8_t array[4]);

void testGpiote(void);

ProtocolErrorCode readBlockPipeOut(void);

ProtocolErrorCode deepFrameToData(Frame *p_frame, uint8_t *p_data, uint16_t *data_len);
void syntheticDataFrame(uint8_t *data);


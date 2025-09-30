
/******************************************************************************
 *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  描述 : 定义了G3无线数据传接口函数
 *  修改记录:
 *
 ******************************************************************************/


#include "G3_Protocol.h"
#include <stdint.h>
#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/sys/byteorder.h>

extern  int frmindex;
extern bool isChannel;
extern uint8_t chans_select;
// static bool isTestMode = false;
// static bool isAppMode = false;
extern struct bt_conn *currentConn;
ProtocolErrorCode dataToFrame(const uint8_t *p_data, uint16_t data_len, Frame *p_frame);
ProtocolErrorCode sendFrame(Frame *p_frame);
ProtocolErrorCode receiveFrame(const uint8_t *p_data, uint16_t data_len);
// void read_conn_rssi(uint16_t handle, int8_t *rssi);
void get_current_conn(struct bt_conn *con);

/**
 * @brief 发送帧
 *
 * @param p_frame
 * @return int
 */
ProtocolErrorCode sendFrame(Frame *p_frame);

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
 * @brief 预修改配置参数
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
 * @brief 流式读取数据
 *
 * @param p_frame
 */
void frmHandlerReadBlockData(Frame *p_frame);

/**
 * @brief 通过引脚控制采集
 *
 * @param p_frame
 */
void frmHandlerSamplingControl(Frame *p_frame);

/**
 * @brief 测试模式与工作模式切换
 *
 * @param p_frame
 */
void frmHandlerChangeMode(Frame *p_frame);

/**
 * @brief 上位机获取电源温度
 *
 * @param p_frame
 */
void frmHandlerGetTemperature(Frame *p_frame);

/**
 * @brief 上位机获取信号质量
 *
 * @param p_frame
 */
void frmHandlerGetRssi(Frame *p_frame);

/**
 * @brief 上位机获取电源电量
 *
 * @param p_frame
 */
void frmHandlerGetSOC(Frame *p_frame);

/**
 * @brief 上位机获取探头电流
 *
 * @param p_frame
 */
void frmHandlerGetProbeCurrent(Frame *p_frame);

/**
 * @brief 上位机获钛壳取温度
 *
 * @param p_frame
 */
void frmHandlerGetShellTemp(Frame *p_frame);

/**
 * @brief 上位机获取ICU电池电压
 *
 * @param p_frame
 */
void frmHandlerGetBatVolt(Frame *p_frame);

/**
 * @brief 上位机获取PTX VDDC电压
 *
 * @param p_frame
 */
void frmHandlerGetPTXVolt(Frame *p_frame);

/**
 * @brief 上位机获取电池充放电电流
 *
 * @param p_frame
 */
void frmHandlerGetBatCurrent(Frame *p_frame);

/**
 * @brief 切换测试模式
 *
 * @param p_frame
 */
void frmHandlerTestMode(Frame *p_frame);

/**
 * @brief 上传指令
 *
 * @param p_frame
 */
void frmHandlerUploadCommond(Frame *p_frame);

/**
 * @brief 通道选择指令
 *
 * @param p_frame
 */
void frmHandlerSelectChannel(Frame *p_frame);

/**
 * @brief FPGA重启指令
 *
 * @param p_frame
 */
void frmHandlerFPGAReset(Frame *p_frame);

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
/**
 * @brief 获取是否是app模式
 *
 */
bool getAppmode();
/**
 * @brief 获取是否是a测试模式
 *
 */
bool getTestMode();
/**
 * @brief 读取数据
 *
 */
ProtocolErrorCode readBlockPipeOut(void);
/**
 * @brief 读取固件版本
 *
 */
void frmHandlerGetVersion(Frame *frame);

/**
 * @brief Set timeout threshold: Timeout to enter sleep mode
 *
 * @param frame
 */
void frmHandlerSetTimeout(Frame *frame);

/**
 * @brief Notify ICU to enter sleep mode
 *
 * @param frame
 */
void frmHandlerCloseICU(Frame *frame);

/**
 * @brief Update ICU Firmware
 *
 * @param p_frame
 */
void frmHandlerUpdateICU_MCU(Frame *p_frame);

/**
 * @brief Sync ICU time
 *
 */
void frmHandlerSyncTime(Frame *p_frame);

/**
 * @brief Get System Event Record
 *
 * @param p_frame
 */
void frmHandlerGetLog(Frame *p_frame);

/**
 * @brief Get State of Health
 *
 * @param p_frame
 */
void frmHandlerGetSOH(Frame *p_frame);

/**
 * @brief Read and write FPGA register
 *
 * @param p_frame
 */
void frmHandlerActFPGAReg(Frame *p_frame);

/**
 * @brief Set sensor limits
 *
 * @param p_frame
 */
void frmHandlerSetSensorLimits(Frame *p_frame);

/**
 * @brief Start impedance measurement
 *
 * @param p_frame
 */
void frmHandlerStartImpedance(Frame *p_frame);

/**
 * @brief Just for Test
 *
 * @param p_frame
 */
void frmHandlerTest(Frame *p_frame);
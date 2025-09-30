/******************************************************************************
 *  版权所有（C）2022-2023，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  描述 : 定义了G3无线通信协议的数据结构
 *  修改记录:
 *
 ******************************************************************************/
#ifndef __G3_PROTOCOL_H__
#define __G3_PROTOCOL_H__

 #include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define HEADER 0xAA55

#pragma pack(1)
typedef struct
{
    uint16_t header;
    uint16_t seq;
    uint8_t type;
    uint16_t dataLen;
    uint8_t data[1];
} Frame;
#pragma pack()

#define FRAME_HEADER_LEN (sizeof(Frame) - 1)
#define DATA_CRC_LEN 1
#define FRAME_HANDLER_NUM (1 << sizeof(uint8_t) * 8)
#define ECU_MODE 1
#define SEND_DATA (247 - FRAME_HEADER_LEN - 1) // 1Byte为数据帧分段标志
// 数据包分段类型
typedef enum
{
    DATA_SEG_FULL = 0x00,
    DATA_SEG_START = 0x01,
    DATA_SEG_MIDDLE = 0x02,
    DATA_SEG_END = 0x03,
} DataSegFlag;

typedef enum
{
    CLOSEICU = 0x00,
    OPENICU = 0x01,
    CLOSEECU = 0x02,
    OPENECU = 0x03,
} ButtonStatus;

// 数据帧类型
typedef enum
{
    FRAME_HEARTBEAT = 0xFF,               // 心跳
    FRAME_STATUS = 0xFE,                  // ICU状态
    FRAME_DEVICE_MODEL = 0x00,            // 获取设备类型
    FRAME_OPEN_DEVICE = 0x01,             // 打开设备
    FRAME_DEVICE_INFO = 0x02,             // 获取设备信息
    FRAME_SET_TIMEOUT = 0x03,             // 设置设备超时时长
    FRAME_WRITE_DATA = 0x04,              // 写入数据
    FRAME_ICU_STATUS = 0x05,              // 获取ICU状态
    FRAME_SET_WIRE_IN_VALUE = 0x06,       // 预修改配置参数
    FRAME_UPDATE_WIRE_IN = 0x07,          // 使配置参数生效
    FRAME_GET_WIRE_IN_VALUE = 0x08,       // 获取配置参数
    FRAME_UPDATE_WIRE_OUT = 0x09,         // 预读取WireOut端点值
    FRAME_GET_WIRE_OUT_VALUE = 0x0A,      // 获取WireOut端点值
    FRAME_ACT_TRIGGER_IN = 0x0B,          // 激活触发器
    FRAME_UPDATE_TRIGGER_OUT = 0x0C,      // 预读取TriggerOut端点值
    FRAME_IS_TRIGGERED = 0x0D,            // 获取触发器状态
    FRAME_READ_DATA = 0x0E,               // 读取数据
    FRAME_WRITE_BLOCK_DATA = 0x0F,        // 写入块形式数据
    FRAME_READ_BLOCK_DATA = 0x10,         // 读取块形式数据
    FRAME_CTRL_DEVICE = 0x11,             // 控制设备
    FRAME_WRITE_DEVICE_ID = 0x12,         // 写入设备ID
    FRAME_UPLOAD_COMMAND = 0x13,          // 上传指令
    FRAME_SAMPLING_CONTROL = 0x14,        // 采集控制
    FRAME_CHANGE_MODE = 0x15,             // 切换测试模式
    FRAME_GET_TEMPERATURE = 0x16,         // 电源温度
    FRAME_GET_RSSI = 0x17,                // 获取信号质量
    FRAME_GET_POWER = 0x18,               // ICU电池获取电量
    FRAME_GET_INTAN_ELECTRUC = 0x19,      // 探头电量
    FRAME_GET_TITANIUM_TEMP = 0x1A,       // 钛壳温度
    FRAME_GET_ICU_BATTERY_VOLTAGE = 0x1B, // ICU电池电压
    FRAME_GET_PTX_VDDC_VOLTAGE = 0x1C,    // PTX VDDC电压
    FRAME_GET_BATTERY_CURRENT = 0x1D,     // 电池充电/放电电流
    FRAME_GET_ICU_CHARGE_STATUS = 0x1E,   // ICU充电状态
    FRAME_EVENT = 0x1F,                   // 事件上报
    FRAME_ERROR = 0x20,                   // 错误上报
    FRAME_TESTMODE = 0x21,                // 测试的几种模式
    FRAME_SELECT_CHANNEL = 0x22,          // 选择通道
    FRAME_UPDATE_ICU_MCU = 0x23,          // ICU_MCU升级
    FRAME_CHARGESTATE = 0x24,             // ECU充电状态，用于485与ECU主控通信
    FRAME_CLOSEICU = 0x25,                // ICU关机
    FRAME_FPGA_RESET = 0x26,              // 重启FPGA
    FRAME_GET_VERSION = 0x28,             // 获取版本号
    FRAME_SET_DEVICE_INFO = 0x29,         // 设置设备信息
    FRAME_GET_HUMIDITY = 0x30,            // 获取温湿度
    FRAME_SEND_PASSWORD = 0x31,           // 发送校验密码
    FRAME_SET_CURRENTTIME = 0x32,         // 时间同步
    FRAME_GET_LOG = 0x33,                 // 获取ICU日志
    FRAME_GET_ICU_SOH = 0x34,             // 获取ICU电池健康程度
    FRAME_START_IMPEDANCES = 0x35,        // 开始阻抗测试
    FRAME_GET_IMPEDANCES_VALUE = 0x36,    // 获取阻抗数据
    FRAME_FPGA_REG = 0x37,                // 操作FPGA寄存器
    FRAME_SET_SENSORDATA = 0x38,          // 设置传感器数据阈值
    FRAME_MOCK_TEST = 0x50,               // 自定义测试指令
    //*< 与STM32交互的指令
    FRAME_MODE_CHANGE = 0x51,
    FRAME_MODE_GET = 0x52,
    FRAME_READ = 0x53,
    FRAME_WRITE = 0x54,
    FRAME_DFU = 0x55,
    FRAME_APP_CRC = 0x56,
    FRAME_VERSION_GET = 0x57,
    FRAME_SENSOR_GET = 0x58,
    FRAME_BLE_STATUS = 0x59,
    FRAME_EVT_NOTICE = 0x5A,
    FRAME_SYNC_LIMIT = 0x5B,
    //*>
} FrameType;

#define HEARTBEAT_INTERVAL 1000 // 心跳包发送间隔，单位ms

// 协议错误码
typedef enum
{
    ERROR_NO_ERROR = 0x00,              // 无错误
    ERROR_INVALID_FRAME = 0x01,         // 无效帧
    ERROR_INVALID_HEADER = 0x02,        // 无效的帧头
    ERROR_INVALID_CRC = 0x03,           // 校验码错误
    ERROR_INVALID_SEQ = 0x04,           // 无效的序列号
    ERROR_INVALID_TYPE = 0x05,          // 无效的类型
    ERROR_INVALID_DATA_SEG_FLAG = 0x06, // 无效的数据段标志
    ERROR_INVALID_DATA_LEN = 0x07,      // 无效的数据长度
    ERROR_INVALID_EP_ADDR = 0x08,       // 无效的端点地址
    ERROR_INVALID_PARAM = 0x09,         // 无效的参数
    ERROR_SEND_FAILED = 0x0A,           // 发送失败
    ERROR_NO_DEVICE_FOUND = 0x0B,       // 未找到设备
    ERROR_OPEN_DEVICE_FAILED = 0x0C     // 打开设备失败

} ProtocolErrorCode;

// 事件码
typedef enum
{
    EVENT_SHUTOFF = 0x00,            // ICU 关机
    EVENT_ICU_OPEN = 0x01,           // ICU 开机
    EVENT_LOWBATT_OFF = 0x02,        // 电池电量低
    EVENT_POWERTMP_HIGH = 0x03,      // 电源温度过高
    EVENT_ICU_ONLINE = 0x04,         // ICU 在线
    EVENT_ICU_OFFLINE = 0x05,        // ICU 离线
    EVENT_ICUCHARGING_ON = 0x06,     // 开始充电
    EVENT_ICUCHARGING_OFF = 0x07,    // 充电结束
    EVENT_CHARGECURRENT_HIGH = 0x08, // 充放电电流过高
    EVENT_INTANCURRENT_HIGH = 0x09,  // 探头电流异常
    EVENT_SHELLTEMP_HIGH = 0x0A,     // 钛壳温度过高
} ProtocolEventCode;

typedef enum
{
    BATTERY_FULL = 0, // 满电
    BATTERY_HIGH = 1, // 高电量
    BATTERY_LOW = 2,  // 低电量
    BATTERY_EMPTY = 3 // 无电量

} Battery;

typedef enum
{
    ECU_WaitForListener=0,  /**< Wait for listener device. */
    ECU_ActivateListener,   /**< This state activates one of multiple listeners found during discovery. */
    ECU_GetCapRecord,       /**< Get capability record state. */
    ECU_WaitListenerReady,  /**< Wait until the listener is ready to accept a charging cycle. */
    ECU_StaticCharging,     /**< Static charging state. */
    ECU_SetInfoRecord,      /**< Set info record state. */
    ECU_GetControlRecord,   /**< Get control record state. */
    ECU_NegoCharging,       /**< Negotiated charging state. */
    ECU_FieldOff,           /**< Field off e.g. due to battery fully charged */
    ECU_DeactivateListener, /**< Deactivate listener state. */
    ECU_PollListener,       /**< Non-blocking wait for LPCD & Discovery. */
    ECU_PollListenerReady,  /**< Non-blocking wait for the listener to accept a charging cycle. */
    ECU_PollCharging,       /**< Non-blocking wait for the WPT to be finished. */
    ECU_PollFieldOff,       /**< Non-blocking wait during RF-field off-duration. */
    ECU_SystemError         /**< System error state --> causes abort of state-machine loop. */
} ChargeState;
#define WIRELESS_BUF_SIZE 1024

#define MAX_UART_FRAME_SIZE 256
#define MAX_BLE_FRAME_SIZE  247
#define COMMON_FRAME_SIZE   64
#define MAX_SEND_DATA_LEN MAX_BLE_FRAME_SIZE - FRAME_HEADER_LEN - 1 // 1为分帧标志
/**
 * @brief 帧处理器
 *
 */
typedef void (*FrameHandler)(Frame *p_frame);

#endif

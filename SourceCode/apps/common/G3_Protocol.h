/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋廷宇 tingyu.song@neuroxess.com
 *  描述 : 定义了G3无线通信协议的数据结构
 *  修改记录:
 *
 ******************************************************************************/
#ifndef __G3_PROTOCOL_H__
#define __G3_PROTOCOL_H__

#include <cstdint>

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

// 数据帧类型
typedef enum
{
    FRAME_HEARTBEAT = 0xFF,
    FRAME_STATUS = 0xFE,

    FRAME_DEVICE_MODEL = 0x00,       // 获取设备类型
    FRAME_OPEN_DEVICE = 0x01,        // 打开设备
    FRAME_DEVICE_INFO = 0x02,        // 获取设备信息
    FRAME_SET_TIMEOUT = 0x03,        // 设置设备超时时长
    FRAME_WRITE_DATA = 0x04,         // 写入数据
    FRAME_ICU_STATUS = 0x05,         // 获取ICU状态
    FRAME_SET_WIRE_IN_VALUE = 0x06,  // 预修改配置参数
    FRAME_UPDATE_WIRE_IN = 0x07,     // 使配置参数生效
    FRAME_GET_WIRE_IN_VALUE = 0x08,  // 获取配置参数
    FRAME_UPDATE_WIRE_OUT = 0x09,    // 预读取WireOut端点值
    FRAME_GET_WIRE_OUT_VALUE = 0x0A, // 获取WireOut端点值
    FRAME_ACT_TRIGGER_IN = 0x0B,     // 激活触发器
    FRAME_UPDATE_TRIGGER_OUT = 0x0C, // 预读取TriggerOut端点值
    FRAME_IS_TRIGGERED = 0x0D,       // 获取触发器状态
    FRAME_READ_DATA = 0x0E,          // 读取数据
    FRAME_WRITE_BLOCK_DATA = 0x0F,   // 写入块形式数据
    FRAME_READ_BLOCK_DATA = 0x10,    // 读取块形式数据
    FRAME_CTRL_DEVICE = 0x11,        // 控制设备
    FRAME_WRITE_DEVICE_ID = 0x12,    // 写入设备ID
    FRAME_UPLOAD_COMMAND = 0x13,     // 上传指令
    FRAME_SAMPLING_CONTROL = 0x14 ,  // 采集控制
		FRAME_CHANGE_MODE = 0x15 ,   // 切换测试模式
		FRAME_GET_TEMPERATURE= 0x16   , // 获取温度
		FRAME_GET_RSSI = 0x17 ,   // 获取信号质量
		FRAME_GET_POWER = 0x18    // 获取电量
} FrameType;

#define HEARTBEAT_INTERVAL 5000 // 心跳包发送间隔，单位ms

// 协议错误码
typedef enum
{
    ERROR_NO_ERROR = 0x00,
    ERROR_INVALID_FRAME = 0x01,
    ERROR_INVALID_HEADER = 0x02,
    ERROR_INVALID_CRC = 0x03,
    ERROR_INVALID_SEQ = 0x04,
    ERROR_INVALID_TYPE = 0x05,
    ERROR_INVALID_DATA_SEG_FLAG = 0x06,
    ERROR_INVALID_DATA_LEN = 0x07,
    ERROR_INVALID_EP_ADDR = 0x08,
    ERROR_INVALID_PARAM = 0x09,
    ERROR_SEND_FAILED = 0x0A,
    ERROR_NO_DEVICE_FOUND = 0x0B,
    ERROR_OPEN_DEVICE_FAILED = 0x0C,
} ProtocolErrorCode;

#define WIRELESS_BUF_SIZE 1024

#define MAX_BLE_FRAME_SIZE 247

/**
 * @brief 帧处理器
 *
 */
typedef void (*FrameHandler)(Frame *p_frame);

#endif

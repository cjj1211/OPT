/******************************************************************************
 *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  修改记录:
 *
 ******************************************************************************/

#ifndef HARDWARE_DEFINE_H
#define HARDWARE_DEFINE_H

#define MAX_DEVICE_ID_LENGTH 32

// USB interface endpoint addresses common to all controller types
typedef enum
{
    WireInResetRun = 0x00,
    WireInSampleRate = 0x02, // 直接设置采样率
    WireInDataFreqPll = 0x03,
    WireInMisoDelay = 0x04,
    WireInChansSetFirst = 0x05, // 前32通道选择
    WireInChansSetLast = 0x06,  // 后32通道选择
    WireInDataStreamEn2 = 0x13, // 数据流使能信号，第33~64个Data Stream,每个芯片对应有2个数据流
    WireInDataStreamEn = 0x14,  // 数据流使能信号，前32个Data Stream,每个芯片对应有2个数据流
    WireInDacSource1 = 0x16,
    WireInDacSource2 = 0x17,
    WireInDacSource3 = 0x18,
    WireInDacSource4 = 0x19,
    WireInDacSource5 = 0x1a,
    WireInDacSource6 = 0x1b,
    WireInDacSource7 = 0x1c,
    WireInDacSource8 = 0x1d,
    WireInDacManual = 0x1e,
    WireInMultiUse = 0x1f,

    TrigInSpiStart = 0x21, // 开始采集
    TrigDirectModeStart = 0x23,
    WireOutVersion = 0x28, // 获取FPGA版本

    WireOutDataClkLocked = 0x24,
    WireOutBoardMode = 0x25,
    WireOutBoardId = 0x3e,
    WireOutBoardVersion = 0x3f,

    WireOutTemperature = 0x26,  // 读取温度传感器的当前值
    WireOutBatteryLevel = 0x27, // 电量
    WireOutSignalLevel = 0x29,  // 获取信号质量
    WireOutFault = 0x2a,        // 获取故障

    PipeOutData = 0xe0,  // 采样数据
    PipeOutImpData = 0xe1,  // 阻抗采样数据

    NotchFilterTypeEP = 0x41, // 陷波器类型，4bit一个Port
    NotchFilterMulti = 0x42,

    LowPassFilterCutFreq_AB = 0x43, // 端口AB的截止频率，16位定义一个端口，最大截止频率65535Hz
    LowPassFilterCutFreq_CD = 0x44, // 端口CD的截止频率
    LowPassFilterCutFreq_EF = 0x45, // 端口EF的截止频率
    LowPassFilterCutFreq_GH = 0x46, // 端口GH的截止频率
    LowPassFilterOrder = 0x47,      // 低通滤波器的阶数，4bit一个Port，1-15，

    HighPassFilterCutFreq_AB = 0x43, // 端口AB的截止频率，16位定义一个端口，最大截止频率65535Hz
    HighPassFilterCutFreq_CD = 0x44, // 端口CD的截止频率
    HighPassFilterCutFreq_EF = 0x45, // 端口EF的截止频率
    HighPassFilterCutFreq_GH = 0x46, // 端口GH的截止频率
    HighPassFilterOrder = 0x47,      // 高通滤波器的阶数，4bit一个Port，1-15

    FilterType = 0x48,             // 滤波器类型设置，4bit一个Port：无滤波、低通、高通、带通
    SignalType = 0x49,             // 信号类型设置，4bit一个Port
    EnableLosslessCompress = 0x4a, // 是否开启无损压缩，1打开，0关闭
    EnableSampleCompress = 0x4b,   // 是否开启采样采样精度压缩，1打开，0关闭

    SpikeThreshold_AB = 0x4c, // 端口AB的Spike门限，16位定义一个端口，最大门限65535微伏
    SpikeThreshold_CD = 0x4d, // 端口CD的Spike门限
    SpikeThreshold_EF = 0x4e, // 端口EF的Spike门限
    SpikeThreshold_GH = 0x4f, // 端口GH的Spike门限
} EndPointAddr;

typedef enum
{
    NF_NONE = 0,
    NF_50Hz,
    NF_60Hz
} NotchFilterType;

typedef enum
{
    ICU_IDLE     = 0x00,
    ICU_SAMPLING = 0x01,
} IcuStatus;

typedef enum
{
    RESET_DEVICE = 0x00,
    POWERR_OFF = 0x01,
} DeviceCtrl;

typedef enum
{
    DEVICE_ICU = 0x00,
    DEVICE_ECU = 0x01,
} DeviceType;

#endif // HARDWARE_DEFINE_H

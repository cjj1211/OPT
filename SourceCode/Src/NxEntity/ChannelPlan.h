/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CHANNELPLAN_H
#define __CHANNELPLAN_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>
#include <QStringList>
#include <QSet>

BEGIN_NX_NAMESPACE

using namespace ormpp;

enum NX_SignalType
{
    NX_Raw = 0,
    NX_LFP,
    NX_Spike
};

struct ChannelPlan :public EntityBase {
    std::string PatientFK;  // 患者表外键
    std::string RecordUidFK; // 训练记录表外键，可空
    std::string Indexs;  // Char 类型
    std::string Position;  // 部位，枚举值
    std::string Area;  // 病区，枚举值
    std::string AreaNum;  // 区号
    std::string DetectorType;  // 探头类型，枚举值
    int  ChannelCounts; //通道总数
    int SampleRate;  // Int 类型
    float LowCut;  // Float 类型
    float HighCut;  // Float 类型
    float HwLowCut{0.1f}; // 硬件低切
    float HwHighCut{500.0f}; // 硬件高切
    float DspCutoff{0.1f};
    std::string Notch;  // 陷波，枚举值
    char EnableDSP{true};
    char EnableLowCut{ true };
    char EnableHighCut{ true };
    std::string SignalType;  // 信号类型，枚举值
    std::string DisabledChannels; // 关闭的通道，保存ChannelNumber列表,以逗号分割，如“0,1”
    char Enabled{true};
    std::string AtlasMapType; // 脑分区地图方式
    int DetectorTranslateX{ 0 };
    int DetectorTranslateY{ 0 };
    int DetectorTranslateZ{ 0 };
    int DetectorRotateX{ 0 };
    int DetectorRotateY{ 0 };
    int DetectorRotateZ{ 0 };
    int ProbeTranslateX{ 0 };
    int ProbeTranslateY{ 0 };
    int ProbeTranslateZ{ 0 };
    int ProbeRotateX{ 0 };
    int ProbeRotateY{ 0 };
    int ProbeRotateZ{ 0 };

    void setEnabled(bool isEnabled) { Enabled = isEnabled; }
    bool isEnabled() { return Enabled; }

    void setDisabledChannelNumbers(QSet<QString> disabledChannels) {
        DisabledChannels = "";
        auto spliter = "";
        for (auto channelNumber : disabledChannels) {
            DisabledChannels.append(spliter + channelNumber.toStdString());
            spliter = ",";
        }
    }

    QSet<QString> getDisabledChannelNumbers() {
        auto disabledChannelNumbers = QString::fromStdString(DisabledChannels).split(',');
        QSet<QString> channelNumbers;
        for (auto channlNumberStr : disabledChannelNumbers) {
            channelNumbers.insert(channlNumberStr);
        }
        return channelNumbers;
    }

    NX_SignalType getSignalType()
    {
        if(QString::fromStdString(SignalType).toUpper() == "RAW")
        {
            return NX_Raw;
        }

        if (QString::fromStdString(SignalType).toUpper() == "LFP")
        {
            return NX_LFP;
        }

        if (QString::fromStdString(SignalType).toUpper() == "SPIKE")
        {
            return NX_Spike;
        }
        return NX_Raw;
    }


    void setSignalType(const NX_SignalType type)
    {
        switch(type)
        {
        case NX_Raw:
            SignalType = "Raw";
            break;
        case NX_LFP:
            SignalType = "LFP";
            break;
        case NX_Spike:
            SignalType = "Spike";
            break;
        default: ;
        }
    }
};

REFLECTION(ChannelPlan, UID, PatientFK, Indexs, Position, Area, AreaNum, DetectorType, SampleRate, LowCut, HighCut, HwLowCut,  
    HwHighCut, DspCutoff, EnableDSP, EnableLowCut, EnableHighCut, Notch, SignalType, UpdateDateTime, IsDeleted, CreateDateTime,   
    ChannelCounts, DisabledChannels, Enabled, RecordUidFK,
    AtlasMapType,
    DetectorTranslateX, DetectorTranslateY, DetectorTranslateZ, DetectorRotateX, DetectorRotateY, DetectorRotateZ,
    ProbeTranslateX, ProbeTranslateY, ProbeTranslateZ, ProbeRotateX, ProbeRotateY, ProbeRotateZ)

END_NX_NAMESPACE
#endif
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __TREATMENTRECORD_H
#define __TREATMENTRECORD_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include"TrainTypeEnum.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>
#include <QCoreApplication>
#include <QString>

BEGIN_NX_NAMESPACE

using namespace ormpp;


struct TreatmentRecord :public  EntityBase {
    std::string PatientFK;  // 患者表外键
    char Score;  // Tinyint 类型
    std::string TrainDataPath;  // Varchar(1024)
    std::string Note; // Varchar(1024)  v0.10.2
    std::string SaveFileType{"rhd"};// “rhd” 或 “edf”，默认为“rhd” v0.10.2
    std::string StartDatetime;  // DateTime 类型
    int TrainTimeLen;  // 训练时长(s)
    char TrainType;  // 训练类型，枚举值
    char IsSuccess;  // 是否成功
    char IsCompress;
    int FailTimes;  // 失败次数
    int SuccessTimes;  // 成功次数
    char Difficult;  // 初始难度

    QString getTrainTypeStr() const
    {
        switch (TrainType)
        {
        case TrainTypeEnum::Click:
            return "Click";
            break;
        case TrainTypeEnum::Move:
            return "Move";
            break;
        case TrainTypeEnum::ProcessTraining:
            return "ProcessTraining";
            break;
        case Action:
            return "Action";
            break;
        default:
            return "Unkonwn";
            break;
        }
        return "";
    }
    
    QString getTimeLenStr() const
    {
        if (TrainTimeLen < 60)
        {
            return QString::number(TrainTimeLen) + "s";
        }
        else if (TrainTimeLen >= 60 && TrainTimeLen < 3600)
        {
            return QString::number(TrainTimeLen / 60) + "min " + QString::number(TrainTimeLen % 60) + "s";
        }
        else {
            const auto hour = TrainTimeLen / 3600;
            const auto minutes = (TrainTimeLen % 3600) / 60;
            const auto secs = TrainTimeLen % 60;
            return QString::number(hour) + "h " + QString::number(minutes) + "min " + QString::number(secs) + "s";
        }
    }
};

REFLECTION(TreatmentRecord, UID, PatientFK, Score, TrainDataPath, Note, SaveFileType, StartDatetime, TrainTimeLen, TrainType, IsSuccess, IsCompress, FailTimes, Difficult, SuccessTimes,UpdateDateTime, IsDeleted, CreateDateTime)

END_NX_NAMESPACE
#endif 
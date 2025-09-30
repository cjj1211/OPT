/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __TREATMENTCFG_H
#define __TREATMENTCFG_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include"TrainTypeEnum.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>
BEGIN_NX_NAMESPACE

using namespace ormpp;

struct TreatmentCfg :public EntityBase {
    std::string PatientFK;  // 患者表外键
    char IsChecked;  // Tinyint 类型
    char TrainType;  // 训练类型，枚举值
    char TrialTime;  // 单次训练时长
    char TrialCounts;  // 训练次数
    char CountDown;  // 倒计时
    char MinScore;  // 最低分
    char MaxScore;  // 最高分
    char Difficult;  // 初始难度
    char MaxDifficult;  // 最高难度  
  
};
REFLECTION(TreatmentCfg, UID, PatientFK, IsChecked, TrainType, TrialTime, CountDown, TrialCounts, MinScore, MaxScore,Difficult,UpdateDateTime, IsDeleted, CreateDateTime, MaxDifficult)

END_NX_NAMESPACE
#endif
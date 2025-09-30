/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __TRAINEVENTS_H
#define __TRAINEVENTS_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include"TrainTypeEnum.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>
BEGIN_NX_NAMESPACE

using namespace ormpp;

struct TrainEvents :public EntityBase {
   
    std::string TreatmentRecordFK;  // 训练记录表外键
    char TrainType;  // 训练类型，枚举值
    char EventType;  // 事件类型，枚举值
    std::string EventDateTime;  // 事件时间点
    char IsSuccess;  // 是否成功
 
};
REFLECTION(TrainEvents, UID, TreatmentRecordFK, TrainType, EventType, EventDateTime, IsSuccess, UpdateDateTime, IsDeleted, CreateDateTime)

END_NX_NAMESPACE
#endif
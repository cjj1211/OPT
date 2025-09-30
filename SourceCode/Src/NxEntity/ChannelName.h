/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CHANNELNAME_H
#define __CHANNELNAME_H

#include "EntityBase.h"
#include <string>
#include <iguana/reflection.hpp>

BEGIN_NX_NAMESPACE

struct ChannelName :public EntityBase {

    std::string PatientFK;  // 患者表外键
    std::string ChannelPlanFK;  // 通道规划表外键
    std::string Indexs;  // 原始名称
    int ChannelCoordX;  // 通道坐标X，Int 类型
    int ChannelCoordY;  // 通道坐标Y，Int 类型
    std::string CustomName;  // 自定义名称
    std::string ChannelNumber; // 从1开始计数，标识程序识别的电极序号
};
REFLECTION(ChannelName, UID, PatientFK, ChannelPlanFK, Indexs, ChannelCoordX, ChannelCoordY, CustomName, UpdateDateTime, IsDeleted, CreateDateTime, ChannelNumber)

END_NX_NAMESPACE
#endif

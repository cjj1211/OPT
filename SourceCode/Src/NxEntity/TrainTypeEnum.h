#/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __TRAINTYPEENUM_H
#define __TRAINTYPEENUM_H


#include "nxentity_global.h"

BEGIN_NX_NAMESPACE

// 定义枚举类型来表示训练类型（TrainType）
enum  TrainTypeEnum {
    Click,
    Move,
    ProcessTraining,
    Action,
    Word
};

END_NX_NAMESPACE
#endif
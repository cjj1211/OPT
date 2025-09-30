/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_MIN_MAX_H
#define __C_MIN_MAX_H
#include "nxwaveformwidget_global.h"
#include <limits>

BEGIN_NX_NAMESPACE

template <class Type>
class CMinMax
{
public:
    CMinMax()
        : minVal(std::numeric_limits<Type>::max())
        , maxVal(std::numeric_limits<Type>::lowest()) { }

    CMinMax(const CMinMax<Type>& init)
        : minVal(init.minVal)
        , maxVal(init.maxVal) { }

    CMinMax& operator=(const CMinMax& init)
    {
        minVal = init.minVal;
        maxVal = init.maxVal;
        return *this;
    }

    CMinMax(Type init)
        : minVal(init)
        , maxVal(init) { }

    Type minVal;
    Type maxVal;

    inline void update(Type value)
    {
        if (value < minVal) minVal = value;
        if (value > maxVal) maxVal = value;
    }
    inline void set(Type value)
    {
        minVal = value;
        maxVal = value;
    }
    inline void reset()
    {
        minVal = std::numeric_limits<Type>::max();
        maxVal = std::numeric_limits<Type>::lowest();
    }
    inline void swap()
    {
        Type temp = minVal;
        minVal = maxVal;
        maxVal = temp;
    }
};

END_NX_NAMESPACE
#endif

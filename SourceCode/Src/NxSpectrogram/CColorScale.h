/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_COLOR_SCALE_H
#define __C_COLOR_SCALE_H
#include "nxspectrogram_global.h"
#include <QPainter>

BEGIN_NX_NAMESPACE

class CColorScale
{
public:
    CColorScale(float minValue_ = 0.0, float maxValue_ = 1.0);
    void setRange(float minValue_, float maxValue_);
    QColor getColor(double value) const;
    void drawColorScale(QPainter& painter, const QRect& r) const;
    void copyColorMapToArray(std::vector<std::vector<float> >& mapArray) const;

private:
    float minValue;
    float maxValue;
    float valueRange;
    std::vector<QColor> colorMap;
    static const int ColorMapSize = 256;

    void calculateColorMap();
};
END_NX_NAMESPACE
#endif
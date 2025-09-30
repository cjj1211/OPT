/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_COORDINATE_TRANSLATOR
#define __C_COORDINATE_TRANSLATOR
#include "nxspectrogram_global.h"
#include <QRect>

BEGIN_NX_NAMESPACE

class CCoordinateTranslator
{
public:
    CCoordinateTranslator();
    CCoordinateTranslator(QRect frame__, double xMinReal__, double xMaxReal__, double yMinReal__, double yMaxReal__);
    void set(QRect frame__, double xMinReal__, double xMaxReal__, double yMinReal__, double yMaxReal__);

    QRect frame() const { return frame_; }
    QRect borderRect() const { return frame_.adjusted(0, 0, -1, -1); }
    QRect clippingRect() const { return frame_.adjusted(1, 1, -1, -1); }
    int xLeft() const { return frame_.left(); }
    int xRight() const { return frame_.right(); }
    int yTop() const { return frame_.top(); }
    int yBottom() const { return frame_.bottom(); }
    double xMinReal() const { return xMinReal_; }
    double xMaxReal() const { return xMaxReal_; }
    double yMinReal() const { return yMinReal_; }
    double yMaxReal() const { return yMaxReal_; }

    int screenXFromRealX(double x) const;
    int screenYFromRealY(double y) const;
    double realXFromScreenX(int x) const;
    double realYFromScreenY(int y) const;

private:
    QRect frame_;
    double xMinReal_;
    double xMaxReal_;
    double yMinReal_;
    double yMaxReal_;
    double xScale;
    double yScale;
};

END_NX_NAMESPACE
#endif
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_COORDINATE_TRANSLATOR_1D_H
#define __C_COORDINATE_TRANSLATOR_1D_H
#include "nxspectrogram_global.h"

BEGIN_NX_NAMESPACE
class CCoordinateTranslator1D
{
public:
    CCoordinateTranslator1D();
    CCoordinateTranslator1D(int minScreen__, int maxScreen__, double minReal__, double maxReal__);
    void set(int minScreen__, int maxScreen__, double minReal__, double maxReal__);

    int minScreen() const { return minScreen_; }
    int maxScreen() const { return maxScreen_; }
    double minReal() const { return minReal_; }
    double maxReal() const { return maxReal_; }

    int screenFromReal(double real) const;
    double realFromScreen(int screen) const;

private:
    int minScreen_;
    int maxScreen_;
    double minReal_;
    double maxReal_;
    double scale;
};

END_NX_NAMESPACE

#endif


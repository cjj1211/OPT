/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_FAST_FOURIER_TRANSFORM_H
#define __C_FAST_FOURIER_TRANSFORM_H
#include "nxspectrogram_global.h"

BEGIN_NX_NAMESPACE
class CFastFourierTransform
{
public:
    enum WindowFunction {
        WindowRectangular,
        WindowTriangular,
        WindowHann,
        WindowHamming
    };
     CFastFourierTransform(float sampleRate_, unsigned int length_ = 1024, WindowFunction function_ = WindowHamming);
     ~CFastFourierTransform();

     void setLength(int length_);
     static void complexInputFft(float* data, unsigned int n);
     static void realInputFft(float* data, unsigned int n);
     float* logSqrtPowerSpectralDensity(float* data);
     float getFrequency(int index) const;

private:
    float sampleRate;
    unsigned int length;
    WindowFunction function;

    float* window;
    float* logPsd;
    float* frequency;

    void createWindow();
    void createPsdVector();
    void createFrequencyVector();
};
END_NX_NAMESPACE
#endif
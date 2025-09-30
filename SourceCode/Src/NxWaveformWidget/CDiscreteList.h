/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DISCRETE_LIST
#define __C_DISCRETE_LIST

#include "nxwaveformwidget_global.h"
#include <QString>

BEGIN_NX_NAMESPACE

struct  Discrete
{
    std::vector<std::string> valueNames;  // e.g. { "1.0", "1" }
    std::string displayedValueName;  // e.g. "1.0 mV"
    double numericValue;
};

class  CDiscreteList
{
public:
    NX_WAVEFORM_WIDGET_EXPORT int getIndex();

    NX_WAVEFORM_WIDGET_EXPORT int getIndex(const std::string& valueName);

    NX_WAVEFORM_WIDGET_EXPORT int shiftIndex(int delta);

    NX_WAVEFORM_WIDGET_EXPORT bool setIndex(int index);

    NX_WAVEFORM_WIDGET_EXPORT double getNumericValue(int index) const;

    NX_WAVEFORM_WIDGET_EXPORT double getNumericValue() const;

    int addItem(const std::string& valueName, const std::string& displayedValueName, double numericalValue = 0.0);

    bool addAlternateValueName(const std::string& valueName, const std::string& alternateValueName);

    int numberOfItems() const;

    bool setValue(const std::string& valueName);

    std::string getValue();

    std::string getValue(int index) const;

    NX_WAVEFORM_WIDGET_EXPORT std::string getDisplayValueString(int index) const;;

    std::string getDisplayValueString() const { return getDisplayValueString(currentIndex); }

    std::vector<Discrete> items;
    int currentIndex;
};

END_NX_NAMESPACE
#endif

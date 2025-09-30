/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DISCRETE_LIST
#define __C_DISCRETE_LIST

#include "nxcommunication_global.h"
#include <QString>

BEGIN_NX_NAMESPACE

struct NXCOMMUNICATION_EXPORT Discrete
{
    std::vector<std::string> valueNames;  // e.g. { "1.0", "1" }
    std::string displayedValueName;  // e.g. "1.0 mV"
    double numericValue;
};

class NXCOMMUNICATION_EXPORT DiscreteList
{
public:
    int addItem(const std::string& valueName, const std::string& displayedValueName, double numericalValue = 0.0);

    bool addAlternateValueName(const std::string& valueName, const std::string& alternateValueName);

    int numberOfItems() const;

    double getNumericValue(int index) const;

    double getNumericValue() const;

    bool setValue(const std::string& valueName);

    std::string getValue();

    std::string getValue(int index) const
    {
        if (index < 0 || index >= (int)items.size()) {
            return "";
        }
        return items[index].valueNames[0];
    }

    std::string getDisplayValueString(int index) const {
        if (index < 0 || index >= static_cast<int>(items.size())) {
            return "";
        }
        return items[index].displayedValueName;
    };

    std::string getDisplayValueString() const { return getDisplayValueString(currentIndex); }

    int getIndex()
    {
        return currentIndex;
    }

    int getIndex(const std::string& valueName)
    {
        const int size = static_cast<int>(items.size());
        for (int i = 0; i < size; ++i) {
            for (auto& name : items[i].valueNames) {
                if (QString::fromStdString(name).toLower() == QString::fromStdString(valueName).toLower()) {
                    return i;
                }
            }
            if (items[i].displayedValueName == valueName) {
                return i;
            }
        }
        return -1;
    }

    int shiftIndex(int delta)
    {
        int newIndex = currentIndex + delta;
        if (newIndex < 0) {
            newIndex = 0;
        }
        else if (newIndex >= numberOfItems()) {
            newIndex = numberOfItems() - 1;
        }
        int actualChange = newIndex - currentIndex;
        setIndex(newIndex);  // Let setCurrentIndex() determine if state needs to signal that it has changed.
        return actualChange;
    }

    bool setIndex(int index)
    {
        if (index < 0 || index >= (int)items.size()) return false;
        if (currentIndex == index) return true;
        currentIndex = index;
        return true;
    }

    std::vector<Discrete> items;
    int currentIndex;
};

END_NX_NAMESPACE
#endif

/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __COLOR_MAP_H
#define __COLOR_MAP_H
#include "nxbrainareawidget_global.h"
#include <iguana/json_reader.hpp>
#include <string>
#include <vector>

struct BrainColor {
    std::vector<double> vect;
};
REFLECTION(BrainColor, vect)

struct ColorReflection {
    int label;
    BrainColor color;
};
REFLECTION(ColorReflection, label, color)

struct ColorMap {
    std::string name;
    std::vector<ColorReflection> relations;
    bool periodic;
};
REFLECTION(ColorMap, name, relations, periodic)

#endif 

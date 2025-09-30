// /******************************************************************************
// *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
// *  作者 : 宋志杰 zhijie.song@neuroxess.com
// *  创建日期:   3 2024
// *  修改记录:
// *  <修改者姓名><邮件地址>   <修改内容>
// ******************************************************************************/

#ifndef ATLAS_CONFIGURATION_H
#define ATLAS_CONFIGURATION_H

#include <QByteArray>

#include "nxbrainareawidget_global.h"
#include <iguana/reflection.hpp>
#include <ylt/struct_json/json_reader.h>

BEGIN_NX_NAMESPACE

struct BrainAreaInfo
{
    std::string Name;
    std::string ChineseName;
    int Label;
    std::vector<int> Center;
    std::vector<int> Rotate;
};

REFLECTION(BrainAreaInfo, Name, ChineseName, Label, Center, Rotate);

struct BrainLobeInfo
{
    std::string Name;
    std::string ChineseName;
    std::vector<BrainAreaInfo> Areas;
};

REFLECTION(BrainLobeInfo, Name, ChineseName, Areas);

struct BrainAtlas
{
    std::vector<BrainLobeInfo> LeftHemisphere;
    std::vector<BrainLobeInfo> RightHemisphere;
};

REFLECTION(BrainAtlas, LeftHemisphere, RightHemisphere);

class NX_BRAIN_AREA_WIDGET_EXPORT CBrainAtlas
{
public:
    static CBrainAtlas& getInstance()
    {
        static CBrainAtlas instance;
        return instance;
    }

    const BrainAtlas& getAtlas() { return atlas; }
private:
    CBrainAtlas()
    {
        const std::string configFilePath = "../config/BrainArea/Desikan-Killiany Atlas.json";
        struct_json::from_json_file(atlas, configFilePath);
    }

    BrainAtlas atlas;

};

#define brainAtlasCfg CBrainAtlas::getInstance()

END_NX_NAMESPACE
#endif

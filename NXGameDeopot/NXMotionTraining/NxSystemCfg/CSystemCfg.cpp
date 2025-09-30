/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CSystemCfg.h"

#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>
#include <QFile>
#include <QString>

BEGIN_NX_NAMESPACE

CSystemCfg& CSystemCfg::Instance(QString filePath)
{
    static CSystemCfg instance(filePath);
    return instance;
}

SystemCfg CSystemCfg::GetSystemCfg()
{
    if (checkSystemCfg(systemCfg))
    {
        return systemCfg;
    }
    else
    {
        return SystemCfg();
    }
}

bool CSystemCfg::UpdateSystemCfg(const SystemCfg& cfg)
{
    systemCfg = cfg;
    std::string jsonDoc;
    struct_json::to_json(systemCfg, jsonDoc);
    auto jsonFile = QFile(configFilePath);
    if (!jsonFile.exists()) {
        LOG_ERR(std::format("File not exist: {}", configFilePath.toStdString()));
        return false;
    }
    if (jsonFile.open(QIODevice::WriteOnly)&& checkSystemCfg(systemCfg)) {
        jsonFile.write(QByteArray::fromStdString(jsonDoc));
        jsonFile.close();
        return true;
    }
    else {
        LOG_ERR(std::format("Open json file fail: {}", configFilePath.toStdString()));
        return false;
    }
}

bool CSystemCfg::checkSystemCfg(const SystemCfg& cfg)
{
    if (systemCfg.Hospital.HospitalName.empty() || systemCfg.Hospital.Department.empty())
    {
        LOG_ERR("Configuration file   struct HospitalCfg lost information ");
        return  false ;
    };
    if (systemCfg.Display.DisplayMode != "dark" && systemCfg.Display.DisplayMode != "light")
    {
        LOG_ERR("Configuration file   struct Display  DisplayMode lost information ");
        return false;
    };
    if (systemCfg.Hardware.CommunicationMode != 0 && systemCfg.Hardware.CommunicationMode != 0 != 1)
    {
        LOG_ERR("Configuration file   struct Hardware  CommunicationMode lost information ");
        return false;
    };
    
    if (!(std::is_same<decltype(systemCfg.Hardware.EnableCompress), bool>::value || std::is_same<decltype(systemCfg.Hardware.EnableFilter), bool>::value  || std::is_same<decltype(systemCfg.Hardware.EnableGPU), bool>::value|| std::is_same<decltype(systemCfg.Hardware.EnableRecord), bool>::value))
    {
        LOG_ERR("Configuration file   struct Hardware   lost information ");
        return false;
    };
    if (!(std::is_same<decltype(systemCfg.Common.EnableTestMode), bool>::value)||systemCfg.Common.Language.empty())
    {
        LOG_ERR("Configuration file   struct Common lost information ");
        return  false;
    }
    return true;
}

CSystemCfg::CSystemCfg(QString filePath)
{
    configFilePath = filePath;
    if (!QFile::exists(filePath))
    {
        LOG_ERR(std::format("File not exist: {}", configFilePath.toStdString()));
        return;
    }

    QByteArray byteArray = configFilePath.toUtf8(); // 使用UTF-8编码将QString转换为QByteArray
    auto path= std::string(byteArray.constData(), byteArray.length());
    struct_json::from_json_file(systemCfg, path);
}


END_NX_NAMESPACE
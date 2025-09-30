/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
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
	return systemCfg;
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
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(QByteArray::fromStdString(jsonDoc));
        jsonFile.close();
        return true;
    }
    else {
        LOG_ERR(std::format("Open json file fail: {}", configFilePath.toStdString()));
        return false;
    }
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
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __C_SYSTEM_CFG_H
#define __C_SYSTEM_CFG_H

#include "../NxMessage/SystemCfg.h"

#include "nxsystemcfg_global.h"
#include <QString>

BEGIN_NX_NAMESPACE

class NXSYSTEMCFG_EXPORT CSystemCfg
{
public:
    static CSystemCfg& Instance(QString filePath = "../config/SystemConfig.json");
    SystemCfg GetSystemCfg();
    bool UpdateSystemCfg(const SystemCfg& cfg);

private:
    CSystemCfg(QString filePath);
    CSystemCfg(const CSystemCfg&) = delete;
    CSystemCfg(const CSystemCfg&&) = delete;
    CSystemCfg& operator =(const CSystemCfg&) = delete;
    CSystemCfg& operator =(const CSystemCfg&&) = delete;

    SystemCfg systemCfg;
    QString configFilePath;

};

#define systemConfig CSystemCfg::Instance()
END_NX_NAMESPACE

#endif

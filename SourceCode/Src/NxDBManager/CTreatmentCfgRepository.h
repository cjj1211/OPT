/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TREATMENTCFG_REPOSITORY_H
#define __C_TREATMENTCFG_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"
#include "../NxEntity/TreatmentCfg.h"
BEGIN_NX_NAMESPACE

class NXDBMANAGER_EXPORT CTreatmentCfgRepository : public IRepository<TreatmentCfg>
{
public:
    CTreatmentCfgRepository();
    virtual ~CTreatmentCfgRepository() override;
    /**

        @fn     getTreatmentCfgByPatient
        @brief  Get treatmentcfg by patient
        @param  patientFK - 
        @param  trainType -  -1代表不查训练类型
        @retval           - treatmentcfg by patient
        @author CuiJunJie
        @date   15.09.2023

    **/
    std::vector<TreatmentCfg> getTreatmentCfgByPatient(const QString& patientFK,int trainType);
};

END_NX_NAMESPACE

#endif

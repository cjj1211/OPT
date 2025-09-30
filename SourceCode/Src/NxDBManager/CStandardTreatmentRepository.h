/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_StandardTreatment_REPOSITORY_H
#define __C_StandardTreatment_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"
#include "../NxEntity/StandardTreatment.h"
BEGIN_NX_NAMESPACE

class NXDBMANAGER_EXPORT CStandardTreatmentRepository : public IRepository<StandardTreatment>
{
public:
    CStandardTreatmentRepository();
    virtual ~CStandardTreatmentRepository() override;

    std::vector< StandardTreatment> getStandardTreatment(QString patientFK, int diseasename, int staging);
    std::vector< StandardTreatment> getStandardTreatmentByPatientFK(QString patientFK);

    void deleteStandardTreatment(QString patientFK);
    void updateStandardTreatment(StandardTreatment standardTreatment);
};
END_NX_NAMESPACE

#endif
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "CTreatmentCfgRepository.h"

BEGIN_NX_NAMESPACE

CTreatmentCfgRepository::CTreatmentCfgRepository()
    :IRepository()
{
}

CTreatmentCfgRepository::~CTreatmentCfgRepository()
{
}

std::vector<TreatmentCfg> CTreatmentCfgRepository::getTreatmentCfgByPatient(const QString& patientFK, int trainType)
{
    QString sql;
   
    if (trainType==-1)
    {
         sql = QString("   IsDeleted = 0 and PatientFK=%1  and trainType= '*'").arg(patientFK);
    }
    else
    {
        sql= QString("  IsDeleted = 0 and PatientFK=%1  and trainType=%2").arg(patientFK).arg(QString::number(trainType));

    }
   
    auto patient = IRepository::db->query<TreatmentCfg>(sql.toStdString());
    if (patient.empty())
    {
        return std::vector<TreatmentCfg>();
    }
    else {
        return patient;
    }
}


END_NX_NAMESPACE



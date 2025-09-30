/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CTreatmentRecordRepository.h"
#include"../NxEntity/TreatmentRecord.h"
BEGIN_NX_NAMESPACE

CTreatmentRecordRepository::CTreatmentRecordRepository()
    :IRepository()
{
}

CTreatmentRecordRepository::~CTreatmentRecordRepository()
{
}

std::vector<TreatmentRecord> CTreatmentRecordRepository::getTreatmentRecordByPatient(const QString& patientFK, const QString& startDatetime, int trainType)
{
    QString sql;

    if (startDatetime.isEmpty()&& trainType == -1)
    {
        sql = QString(" IsDeleted = 0 and PatientFK='%1' ").arg(patientFK);
    }
    else if(startDatetime.isEmpty() && trainType != -1)
    {
        sql = QString("   IsDeleted = 0 and PatientFK='%1'   and  trainType=%2 ").arg(patientFK).arg(QString::number(trainType));
    }
    else if (trainType == -1)
    {
        sql = QString("   IsDeleted = 0 and PatientFK='%1' and StartDatetime='%2'  and trainType IS NOT NULL").arg(patientFK).arg(startDatetime);
    }

    else
    {
        sql = QString("  IsDeleted = 0 and PatientFK='%1'  and trainType=%2 and StartDatetime='%3'").arg(patientFK).arg(QString::number(trainType)).arg(startDatetime);

    }

    auto treatmentRecord = IRepository::db->query<TreatmentRecord>(sql.toStdString());
    if (treatmentRecord.empty())
    {
        return std::vector<TreatmentRecord>();
    }
    else {
        return treatmentRecord;
    }
}
std::vector<TreatmentRecord> CTreatmentRecordRepository::getTreatmentRecordByUpateTime()
{
   
    auto treatmentRecord = IRepository::db->query<TreatmentRecord>("where IsDeleted = 0 order by UpdateDateTime  desc limit 5 offset 0 ");
    if (treatmentRecord.empty())
    {
        return std::vector<TreatmentRecord>();
    }
    else {
        return treatmentRecord;
    }
}


END_NX_NAMESPACE



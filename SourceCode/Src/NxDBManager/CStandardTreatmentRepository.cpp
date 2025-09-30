/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "CStandardTreatmentRepository.h"

BEGIN_NX_NAMESPACE

CStandardTreatmentRepository::CStandardTreatmentRepository()
    :IRepository()
{
}

CStandardTreatmentRepository::~CStandardTreatmentRepository()
{


}

std::vector<StandardTreatment> CStandardTreatmentRepository::getStandardTreatment(QString patientFK, int diseasename, int staging)
{

    QString sql = QString("   IsDeleted = 0 and PatientFK='%1'  and Diseasename = %2 and Staging= %3 ").arg(patientFK, QString::number(diseasename), QString::number(staging));

    auto standardTreatment = IRepository::db->query<StandardTreatment>(sql.toStdString());
    if (standardTreatment.empty())
    {
      
           
            return  standardTreatment;
      
    }
    else {
        return standardTreatment;
    }

}

std::vector<StandardTreatment> CStandardTreatmentRepository::getStandardTreatmentByPatientFK(QString patientFK)
{
    QString sql = QString("   IsDeleted = 0 and PatientFK=%1  ").arg(patientFK);

    auto standardTreatment = IRepository::db->query<StandardTreatment>(sql.toStdString());
    if (standardTreatment.empty())
    {
        
        return  standardTreatment;
    }
    else {
        return standardTreatment;
    }
}

void CStandardTreatmentRepository::deleteStandardTreatment(QString patientFK)
{
    QString sql = QString("   IsDeleted = 0 and PatientFK=%1  ").arg(patientFK);

    auto standardTreatment = IRepository::db->query<StandardTreatment>(sql.toStdString());
  
    for (auto  standard: standardTreatment)
    {
        standard.IsDeleted = true;
        IRepository::db->update(standard);
    }
}
void CStandardTreatmentRepository::updateStandardTreatment(StandardTreatment standardTreatment)
{
    IRepository::db->update(standardTreatment);
}
END_NX_NAMESPACE



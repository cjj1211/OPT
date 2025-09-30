/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_PATIENT_REPOSITORY_H
#define __C_PATIENT_REPOSITORY_H

#include "nxdbmanager_global.h"
#include"../NxEntity/Patient.h"
#include "IRepository.h"

BEGIN_NX_NAMESPACE
struct Patient;
struct IdentityInfo;

class NXDBMANAGER_EXPORT CPatientRepository : public IRepository<Patient>
{
public:
    CPatientRepository();
    virtual ~CPatientRepository() override;

    bool createPatient(const IdentityInfo& identityInfo, const Patient& patient);

    bool updatePatient(const IdentityInfo& identityInfo, const Patient& patient);

    int getPatientCount(QString filter);

    Patient getPatientByIdentifyFK(const QString & identifyFK);

    std::vector<Patient> getPatientByPatientName(const QString& patientName);

    bool getPatientByCaseId(const QString& caseId, Patient& patient);

    std::vector<Patient> getPatientByGender(GenderEmu gender);

    std::vector<Patient> getPatientByBirthday(const QString& birthday);

    std::vector<Patient> getPatientCurrentPage(int currentpage,int number, QString filter);

    bool isNameDuplicate(const QString& patientName, const QString& uid);

    bool isCaseIdDuplicate(const QString& caseId, const QString& uid);

    bool deleteWithRefInfoByUid(const QString& uid);
};

END_NX_NAMESPACE

#endif

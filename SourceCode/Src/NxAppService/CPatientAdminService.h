/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __PATIENT_ADMIN_SERVICE_H
#define __PATIENT_ADMIN_SERVICE_H
#include "nxappservice_global.h"
#include "IAppService.h"
#include "NxFaceIdentifier/CFeatureDetector.h"
#include "NxFaceIdentifier/CFaceIdentifier.h"
#include <QImage>

namespace NX
{
    class CTreatmentRecordRepository;
}

namespace NX
{
    class CChannelPlanRepository;
}

class QImage;

BEGIN_NX_NAMESPACE
struct Patient;
struct IdentityInfo;
enum GenderEmu;
class CPatientRepository;
class CIdentifyInfoRepository;

class NX_APP_SERVICE_EXPORT CPatientAdminService
    :public IAppService
{
    Q_OBJECT
public:
    CPatientAdminService(QObject* parent = nullptr);
    
    virtual ~CPatientAdminService() override;

    bool createPatient(const IdentityInfo& identityInfo, const Patient& patient) const;

    bool updatePatient(const IdentityInfo& identityInfo, const Patient& patient) const;

    bool isNameDuplicate(const QString& patientName, const QString& uid) const;

    bool isCaseIdDuplicate(const QString& caseId, const QString& uid) const;

    int getPatientCount(const QString& filter) const;

    bool getPatientIdentifyInfo(const QString& uid , IdentityInfo& identityInfo) const;

    std::vector<Patient> getPatientByPatientName(const QString& patientName) const;

    bool getPatientByUid(const QString& uid, Patient& patient) const;

    bool getPatientByCaseId(const QString& caseId, Patient& patient) const;

    std::vector<Patient> getPatientByBirthday(const QString& birthday) const;

    std::vector<Patient> getPatientCurrentPage(int currentPage, int number, const QString& filter) const;

    bool deletePatientByUid(const QString& patientUid) const;

private:
    std::unique_ptr<CPatientRepository> patientRepository;
    std::unique_ptr<CIdentifyInfoRepository> identityInfoRepository;
};

END_NX_NAMESPACE

#endif
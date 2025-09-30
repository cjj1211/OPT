#include "CPatientAdminService.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxDBManager/CIdentifyInfoRepository.h"

BEGIN_NX_NAMESPACE

CPatientAdminService::CPatientAdminService(QObject* parent)
	:IAppService(parent)
	, patientRepository(new CPatientRepository())
	, identityInfoRepository(new CIdentifyInfoRepository())
{
}

CPatientAdminService::~CPatientAdminService()
{
}

bool CPatientAdminService::createPatient(const IdentityInfo& identityInfo, const Patient& patient) const
{
	return patientRepository->createPatient(identityInfo, patient);
}

bool CPatientAdminService::updatePatient(const IdentityInfo& identityInfo, const Patient& patient) const
{
	return patientRepository->updatePatient(identityInfo, patient);
}

bool CPatientAdminService::isNameDuplicate(const QString& patientName, const QString& uid) const
{
	return patientRepository->isNameDuplicate(patientName, uid);
}

bool CPatientAdminService::isCaseIdDuplicate(const QString& caseId, const QString& uid) const
{
	return patientRepository->isCaseIdDuplicate(caseId, uid);
}

int CPatientAdminService::getPatientCount(const QString& filter) const
{
	return patientRepository->getPatientCount(filter);
}

bool CPatientAdminService::getPatientIdentifyInfo(const QString& uid, IdentityInfo& identityInfo) const
{
	return identityInfoRepository->GetByUID(uid, identityInfo);
}

std::vector<Patient> CPatientAdminService::getPatientByPatientName(const QString& patientName) const
{
	return patientRepository->getPatientByPatientName(patientName);
}

bool CPatientAdminService::getPatientByUid(const QString& uid, Patient& patient) const
{
	return patientRepository->GetByUID(uid, patient);
}

bool CPatientAdminService::getPatientByCaseId(const QString& caseId, Patient& patient) const
{
	return patientRepository->getPatientByCaseId(caseId, patient);
}

std::vector<Patient> CPatientAdminService::getPatientByBirthday(const QString& birthday) const
{
	return patientRepository->getPatientByBirthday(birthday);
}

std::vector<Patient> CPatientAdminService::getPatientCurrentPage(const int currentPage, const int number, const QString& filter) const
{
	return patientRepository->getPatientCurrentPage(currentPage, number, filter);
}

bool CPatientAdminService::deletePatientByUid(const QString& patientUid) const
{
	return  patientRepository->deleteWithRefInfoByUid(patientUid);
}
END_NX_NAMESPACE



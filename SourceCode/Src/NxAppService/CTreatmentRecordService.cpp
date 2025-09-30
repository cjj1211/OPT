
#include "CTreatmentRecordService.h"
#include "NxDBManager/CTreatmentRecordRepository.h"
#include "NxEntity/TreatmentRecord.h"
BEGIN_NX_NAMESPACE

CTreatmentRecordService::CTreatmentRecordService(const QString& patientUid, QObject* parent)
	: IAppService(parent)
	, treatmentRepository(std::make_unique<CTreatmentRecordRepository>())
	, currentPatientUid(patientUid)
{
}

CTreatmentRecordService::~CTreatmentRecordService()
{
}

std::vector<TreatmentRecord> CTreatmentRecordService::getTreatmentRecordByUpdateTime() const
{
	return treatmentRepository->getTreatmentRecordByUpateTime();
}

std::vector<TreatmentRecord> CTreatmentRecordService::getTreatments() const
{
	return treatmentRepository->getTreatmentRecordByPatient(currentPatientUid, "", -1);
}

bool CTreatmentRecordService::deleteTreatmentByUid(const QString& uid) const
{
	return treatmentRepository->DeleteByUID(uid);
}

bool CTreatmentRecordService::updateTreatmentRecord(const TreatmentRecord& record) const
{
	return treatmentRepository->Update(record);
}

END_NX_NAMESPACE


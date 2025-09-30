
#include "CStandardTreatmentService.h"
#include "NxDBManager/CStandardTreatmentRepository.h"
#include "NxEntity/StandardTreatment.h"

BEGIN_NX_NAMESPACE

CStandardTreatmentService::CStandardTreatmentService(const QString& patientUid, QObject* parent )
	: IAppService(parent),
	standardTreatmentRepository(std::make_unique<CStandardTreatmentRepository>()),
	currentPatientUid(patientUid)
{
}

CStandardTreatmentService::~CStandardTreatmentService()
{

}

std::vector<StandardTreatment> CStandardTreatmentService::getStandardTreatment(int diseasename, int staging)
{
	return standardTreatmentRepository->getStandardTreatment(currentPatientUid, diseasename, staging);
}

std::vector<StandardTreatment> CStandardTreatmentService::getStandardTreatmentByPatientFK()
{
	return standardTreatmentRepository->getStandardTreatmentByPatientFK(currentPatientUid);
}

void CStandardTreatmentService::deleteStandardTreatment()
{
	standardTreatmentRepository->deleteStandardTreatment(currentPatientUid);
}

void CStandardTreatmentService::updateStandardTreatment(StandardTreatment standardTreatment)
{
	standardTreatmentRepository->updateStandardTreatment(standardTreatment);
}



END_NX_NAMESPACE


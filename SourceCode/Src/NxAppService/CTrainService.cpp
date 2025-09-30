#include "CTrainService.h"

#include "NxDBManager/CChannelPlanRepository.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "NxDBManager/CPatientRepository.h"
#include "NxDBManager/CIdentifyInfoRepository.h"

BEGIN_NX_NAMESPACE

CTrainService::CTrainService(QObject* parent)
    : IAppService(parent)
    , patientRepository(new CPatientRepository)
    , identityInfoRepository(new CIdentifyInfoRepository)
    , channelPlanRepository(new CChannelPlanRepository)
{}

CTrainService::~CTrainService() = default;

void CTrainService::getPatientInfo(const QString& uid, Patient& patientInfo) const
{
    patientRepository->GetByUID(uid, patientInfo);
}

void CTrainService::getPatientIdentifyInfo(const QString& uid, IdentityInfo& identityInfo) const
{
    identityInfoRepository->GetByUID(uid, identityInfo);
}

std::vector<ChannelPlan> CTrainService::getChannelPlans(const QString& patientUid) const
{
    return channelPlanRepository->getChannelPlanByPatient(patientUid);
}

END_NX_NAMESPACE

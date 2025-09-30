#include "CPlaybackService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxDBManager/CChannelPlanRepository.h"
#include "NxDBManager/CChannelNameRepository.h"


BEGIN_NX_NAMESPACE

CPlaybackService::CPlaybackService(const QString& patientUid, const QString& _recordUid, QObject *parent)
    : COscillographService(patientUid, parent)
{
    recordUid = _recordUid;
    recorderType = RT_PLAYBACK;
}

CPlaybackService::~CPlaybackService()
{}

std::vector<ChannelPlan> CPlaybackService::getChannelPlans() const
{
    return channelPlanRepository->getByPatientAndRecordUID(currentPatientUid, recordUid);
}

END_NX_NAMESPACE
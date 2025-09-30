
#include "CChannelPlanService.h"
#include "NxDBManager/CChannelPlanRepository.h"
#include "NxEntity/ChannelPlan.h"
BEGIN_NX_NAMESPACE

CChannelPlanService::CChannelPlanService(QObject* parent)
	: IAppService(parent)
	, channelRepository(std::make_unique<CChannelPlanRepository>())
{
}

CChannelPlanService::~CChannelPlanService()
{
}

std::vector<ChannelPlan> CChannelPlanService::getChannelPlanByPatient(const QString& patientUid)
{
	return channelRepository->getChannelPlanByPatient(patientUid);
}

void CChannelPlanService::deleteChannelPlanByIndexs(const QString& indexs, const QString& patientUid)
{
	channelRepository->deleteChannelPlanByIndexs(indexs, patientUid);
}
void CChannelPlanService::updateChannelPlan(std::vector<ChannelPlan> channels)
{
	channelRepository->updateChannelPlan(channels);
}

ChannelPlan  CChannelPlanService::getChannelPlanByIndexs(const QString& patientUid, const QString& indexs)
{
	return channelRepository->getChannelPlanByIndexs(patientUid, indexs);
}
ChannelPlan CChannelPlanService::getChannelPlanByDetectorType(const QString& patientUid, int detectorType)
{

	return channelRepository->getChannelPlanByDetectorType(patientUid, detectorType);
}
ChannelPlan CChannelPlanService::getChannelPlanByUid(const QString& uid)
{
	ChannelPlan channel= channelRepository->getChannelPlanByUid(uid);

	if (channel.UID!="")
	{
		return channel;
	}
	return ChannelPlan();
}

void CChannelPlanService::insertChannelPlan(ChannelPlan channels)
{
	channelRepository->insertChannelPlan(channels);
}
END_NX_NAMESPACE


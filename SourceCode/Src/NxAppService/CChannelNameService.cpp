
#include "CChannelNameService.h"
#include "NxDBManager/CChannelNameRepository.h"
#include "NxEntity/ChannelName.h"
BEGIN_NX_NAMESPACE

CChannelNameService::CChannelNameService(QObject* parent)
	: IAppService(parent)
	, channelNameRepository(std::make_unique<CChannelNameRepository>())
{
}

CChannelNameService::~CChannelNameService()
{
}

QPair<int, int> CChannelNameService::getChannelPositon(const QString& fk,const QString& CustomName)
{
	return channelNameRepository->getChannelPositon(fk,CustomName);
}
void CChannelNameService::deleteChannelBychannelFK(const QString& fk)
{
	channelNameRepository->deleteChannelBychannelFK(fk);
}
std::vector<ChannelName> CChannelNameService::getChannelBychannelFK(const QString& fk)
{
	return channelNameRepository->getChannelNameByChannelPlanFK(fk);
}

//ChannelName CChannelNameService::getChannelBychannelPosition(const int x, const int y)
//{
//	return channelNameRepository->getChannelBychannelPosition(x,y);
//}
ChannelName    CChannelNameService::getChannelBychannelPosition(const int x, const int y, const QString& index, const QString& fk)
{
	return channelNameRepository->getChannelBychannelPosition(x, y, index, fk);
}
void CChannelNameService::updateChannelName(std::vector<ChannelName> channelname)
{
	channelNameRepository->updateChannelName(channelname);

}
void CChannelNameService::updateChannelName(ChannelName channelname)
{
	channelNameRepository->updateChannelName(channelname);
}
END_NX_NAMESPACE


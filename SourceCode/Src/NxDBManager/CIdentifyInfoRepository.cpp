#include "CIdentifyInfoRepository.h"

BEGIN_NX_NAMESPACE
CIdentifyInfoRepository::CIdentifyInfoRepository()
{
}

CIdentifyInfoRepository::~CIdentifyInfoRepository() = default;

IdentityInfo CIdentifyInfoRepository::getIdentityInfoByUid(const QString& uid)
{
    auto infoRepository = db->query<IdentityInfo>(" UID ='"+uid.toStdString() +"'" +" and IsDeleted = 0");
	if (infoRepository.empty())
	{
		return IdentityInfo();
	}
	else
	{
		return infoRepository[0];
	}
}

std::vector<IdentityInfo> CIdentifyInfoRepository::getIdInfoByLoginName(const QString& loginName)
{
	auto infoRepository = db->query<IdentityInfo>(" LoginName = '" + loginName.toStdString()+"'" + " and IsDeleted = 0");
	if (infoRepository.empty())
	{
		return {};
	}
	else
	{
		return infoRepository;
	}
}

std::vector<IdentityInfo> CIdentifyInfoRepository::getAllIdInfos()
{
	auto infoRepository = db->query<IdentityInfo>(" IsDeleted = 0");
	if (infoRepository.empty())
	{
		return {};
	}
	else
	{
		return infoRepository;
	}
}

END_NX_NAMESPACE

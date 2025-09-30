#include "IAppService.h"
#include "NxDBManager/CPatientRepository.h"
#include <QDebug>

#include "NxEntity/IdentityInfo.h"
#include "NxSystemCfg/CSystemCfg.h"

BEGIN_NX_NAMESPACE
bool IAppService::hasLogin = false;
std::string IAppService::currentLoginName;
std::string IAppService::currentIdInfoUid;
std::string IAppService::currentPatientUid;
RoleType IAppService::currentRole = Role_User;

IAppService::IAppService(QObject* parent)
	: QObject(parent)
{
    const auto config = systemConfig.GetSystemCfg().Common;	
	CDBManager::initDB(config.DBIp, config.DBUser, config.DBPassword, config.DBName, config.DBPort);

}

IAppService::~IAppService() = default;

void IAppService::setLoginInfo(const QString& loginName, const QString& infoUid, const RoleType role)
{
	currentLoginName = loginName.toStdString();
	currentIdInfoUid = infoUid.toStdString();
	currentRole = role;
}

void IAppService::setLoginRoleType(const RoleType role)
{
	currentRole = role;
}

void IAppService::setCurrentPatientUid(const QString& patientUid)
{
    currentPatientUid = patientUid.toStdString();
}

END_NX_NAMESPACE

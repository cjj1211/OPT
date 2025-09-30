#include "CRestoreService.h"
#include "NxSystemCfg/CSystemCfg.h"
BEGIN_NX_NAMESPACE
CRestoreService::CRestoreService(QObject *parent)
	: IAppService(parent)
{
	auto commonCfg = systemConfig.GetSystemCfg().Common;

	backupResClient = std::make_unique<CBackupResClient>(commonCfg.RecorderAddress);
}

CRestoreService::~CRestoreService()
{


}

void CRestoreService::setRestore(QVector<QString> restorePath)
{
	backupResClient->setRestore(restorePath);
}
END_NX_NAMESPACE

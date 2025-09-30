#pragma once
#include "nxappservice_global.h"
#include "NxCommunication/CBackupResClient.h"

#include "IAppService.h"
#include <QObject>
#include <QVector>
BEGIN_NX_NAMESPACE

class NX_APP_SERVICE_EXPORT CRestoreService  : public IAppService
{
	Q_OBJECT

public:
	CRestoreService(QObject *parent = nullptr);
	~CRestoreService();
	void setRestore(QVector<QString> restorePath);
private:
	std::unique_ptr<CBackupResClient>backupResClient ;
};
END_NX_NAMESPACE
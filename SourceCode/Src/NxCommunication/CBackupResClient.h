#ifndef CBackup_ResClient_H
#define CBackup_ResClient_H


#include <string>

#include "nxcommunication_global.h"
BEGIN_NX_NAMESPACE

class NXCOMMUNICATION_EXPORT CBackupResClient
{


public:
	CBackupResClient(const std::string &  address);
	~CBackupResClient();
	void setRestore(QVector<QString> restorePath);

private:
	class CBackupResClientImpl;
	CBackupResClientImpl* impl;
};
END_NX_NAMESPACE
#endif

#pragma once

#include <QObject>

class BackupResClient  : public QObject
{
	Q_OBJECT

public:
	BackupResClient(const std::string &  address);
	~BackupResClient();
	void setBackup( const QString& backupdate);

private:
	class BackupResServiceImpl;
	BackupResServiceImpl* impl;
};

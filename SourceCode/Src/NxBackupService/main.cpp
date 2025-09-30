
#include "CBackupDataBase.h"
#include "BackupResImpl.h"
#include "BackupResClient.h"
#include <QDateTime>
int main()
{
	CBackupDataBase backup(nullptr);
	const std::unique_ptr<BackupResClient>backupResClient = std::make_unique<BackupResClient>(backup.recorderAddress);
	const QString backdate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
	backupResClient->setBackup(backdate);
	return 0;
}

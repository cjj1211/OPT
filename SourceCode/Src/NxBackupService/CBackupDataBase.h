#pragma once

#include <QObject>
#include <QProcess>
#include "../NxEntity/SystemCfg.h"
class CBackupDataBase  : public QObject
{
	Q_OBJECT

public:
	CBackupDataBase(QObject *parent);
	~CBackupDataBase();
public :
std::string recorderAddress = "127.0.0.1:50061";
private:
	void executeBackUpBate();
private :
	CommonCfg common;
	QProcess* process ;
};

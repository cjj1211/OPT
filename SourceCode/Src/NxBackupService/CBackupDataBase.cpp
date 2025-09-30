#include "CBackupDataBase.h"
#include "../NxSystemCfg/CSystemCfg.h"
#include <CLogger.h>
#include <QDateTime>
using namespace NX;
CBackupDataBase::CBackupDataBase(QObject *parent)
	: QObject(parent)
{
    process = new QProcess();
	common = systemConfig.GetSystemCfg().Common;
    recorderAddress = common.RecorderAddress;
	executeBackUpBate();

}

CBackupDataBase::~CBackupDataBase()
{

    delete process;
}

void CBackupDataBase::executeBackUpBate()
{
    LOG_INFO("Start Backup DB");
    QDateTime targetTime = QDateTime::currentDateTime();
    QDateTime beginTime = QDateTime::currentDateTime().addDays(-7*( common.BackupDate));
    QString backtime =  beginTime.toString("yyyy-MM-dd") + " -----" + targetTime.toString("yyyy-MM-dd");
    LOG_INFO(backtime.toStdString());
    QString program = "../config/InsertBackUp.bat"; // 替换成你的批处理文件路径
    program = program.replace("/", "\\");

    QString  sqlPath = QString::fromStdString(common.BackupPath)  + beginTime.toString("yyyy-MM-dd") + "-" + targetTime.toString("yyyy-MM-dd") + ".sql";
    sqlPath= sqlPath.replace("/", "\\");
    QStringList arguments;
    arguments << QString::fromStdString(common.DBUser)<<QString::fromStdString(common.DBPassword)<< QString::fromStdString(common.DBName)<< beginTime.toString("yyyy-MM-dd")<< targetTime.toString("yyyy-MM-dd") << sqlPath; // 传递给批处理文件的参数
    process->start(program, arguments);
    process->waitForFinished(-1);  // 等待进程完成

    LOG_INFO("End Backup DB");
}

#ifndef CSYSTEMSETTINGS_H
#define CSYSTEMSETTINGS_H

#include <QWidget>
#include "../Global.h"
#include <QTranslator>
#include "ui_CSystemSettings.h"
#include "../../NxEntity/User.h"
#include "../../NxEntity/IdentityInfo.h"
#include "../../NxEntity/SystemCfg.h"


BEGIN_NX_NAMESPACE

class COscillographService;
class CUserInfo;
class CUserService;
struct IdentityInfo;
struct User;
class CSwitchButton;
class CSystemCfg;
class CProgressDialog;
struct SQLInfo
{
    QString tableName="";
    QString uid="";
    QString updateTime="";

};
class CSystemSettings : public QWidget
{
    Q_OBJECT

public:
    CSystemSettings(QWidget *parent = nullptr);
    ~CSystemSettings();
private:
    void setCheck();
    void initWidget();
    void initSysConfig();
    void getStorageInfoList();
    QString formatBytes(qint64 bytes);
    void analyticalSql(const QString& fileName);
    void getSqlInfo(const QString& sqlStatement);
private slots:
    void slotUserManageClick();
    void slotHospitalInfoClick();
    void slotDropboxClick();
    void slotBackupRestoreClick();
    void slotCreateUser();
    void slotEditUser();
    void slotDeleteUser();
    void slotCloseUserInfo();
    void updateSysConfig();
    void slotBackupClick();
    void slotRestoreClick();

     

private:
    std::unique_ptr<Ui::CSystemSettings> ui;
    CUserInfo* userInfo;
    CUserService* userService;

    COscillographService* oscillographService;
    CSwitchButton* gpuSwitchButton;
    CSwitchButton* compressSwitchButton;
    IdentityInfo currentIn;
    User currentuser;
    SystemCfg systemCfg;
    QStringList fileList;
    CProgressDialog* progressDialog;
    
    
    MYSQL mysqlInstance;
    SQLInfo sqlInfo;
    MYSQL_RES* result;

};
#endif // CSYSTEMSETTINGS_H
END_NX_NAMESPACE
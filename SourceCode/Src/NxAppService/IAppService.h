/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __I_APP_SERVICE_H
#define __I_APP_SERVICE_H
#include "nxappservice_global.h"
#include <QObject>

#include "NxEntity/BasicTypes.h"

BEGIN_NX_NAMESPACE

class NX_APP_SERVICE_EXPORT IAppService: public QObject
{
    Q_OBJECT

public:
    explicit IAppService(QObject* parent = nullptr);

    virtual  ~IAppService() override;

    static bool isLogin() { return hasLogin; }

    static void setLoginInfo(const QString& loginName, const QString& infoUid, RoleType role);

    static void setLoginRoleType(RoleType role);

    static void setCurrentPatientUid(const QString& patientUid);

    static QString getCurrentLoginName() { return QString::fromStdString(currentLoginName); }

    static QString getCurrentPatientUid() { return QString::fromStdString(currentPatientUid); }

    static QString getCurrentIdentityUid() { return QString::fromStdString(currentIdInfoUid); }

    static RoleType getCurrentRole() { return currentRole; }

private:
    static bool hasLogin;
    static std::string currentLoginName;
    static std::string currentIdInfoUid;
    static std::string currentPatientUid;
    static RoleType currentRole;


};


END_NX_NAMESPACE

#endif


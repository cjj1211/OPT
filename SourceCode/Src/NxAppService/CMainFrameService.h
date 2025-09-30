/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef _C_MAIN_FRAME_SERVICE_H
#define _C_MAIN_FRAME_SERVICE_H

#include "IAppService.h"

BEGIN_NX_NAMESPACE
struct Patient;
struct User;
struct IdentityInfo;
class CUserRepository;
class CIdentifyInfoRepository;
class CPatientRepository;

class NX_APP_SERVICE_EXPORT CMainFrameService: public IAppService
{
    Q_OBJECT
public:
    CMainFrameService(QObject* parent = nullptr);
    ~CMainFrameService() override;

    User getUserByIdentityFK(const QString& identityFK) const;
    IdentityInfo getIdentityByUid(const QString& uid) const;
    bool getUserByUid(const QString& uid, User& user) const;
    bool getPatientByUid(const QString& uid, Patient& patient) const;

private:
    std::unique_ptr<CPatientRepository> patientRepository;
    std::unique_ptr<CIdentifyInfoRepository> identityInfoRepository;
    std::unique_ptr<CUserRepository> userRepository;
};

END_NX_NAMESPACE
#endif
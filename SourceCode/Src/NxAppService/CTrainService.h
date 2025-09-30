/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   December 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TRAIN_SERVICE_H
#define __C_TRAIN_SERVICE_H
#include "nxappservice_global.h"
#include "IAppService.h"
#include "NxEntity/ChannelPlan.h"

namespace NX
{
    class CChannelPlanRepository;
}

BEGIN_NX_NAMESPACE

class CPatientRepository;
class CIdentifyInfoRepository;
struct Patient;
struct IdentityInfo;
class NX_APP_SERVICE_EXPORT CTrainService : public IAppService
{
    Q_OBJECT

public:
    CTrainService(QObject* parent = nullptr);
    ~CTrainService() override;

    void getPatientInfo(const QString& uid, Patient& patientInfo) const;
    void getPatientIdentifyInfo(const QString& uid, IdentityInfo& identityInfo) const;
    std::vector<ChannelPlan> getChannelPlans(const QString& patientUid) const;
private:
    std::unique_ptr<CPatientRepository> patientRepository;
    std::unique_ptr<CIdentifyInfoRepository> identityInfoRepository;
    std::unique_ptr<CChannelPlanRepository> channelPlanRepository;
};

END_NX_NAMESPACE
#endif

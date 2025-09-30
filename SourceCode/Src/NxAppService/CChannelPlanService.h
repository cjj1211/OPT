/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CHANNEL_PLAN_SERVICE_H
#define __CHANNEL_PLAN_SERVICE_H

#include "nxappservice_global.h"

#include "IAppService.h"

BEGIN_NX_NAMESPACE

class CChannelPlanRepository;

struct ChannelPlan;
class NX_APP_SERVICE_EXPORT CChannelPlanService
    :public IAppService
{
    Q_OBJECT
public:
    CChannelPlanService(QObject* parent = nullptr);
    virtual ~CChannelPlanService() override;
    std::vector<ChannelPlan> getChannelPlanByPatient(const QString& patientUid);
    void deleteChannelPlanByIndexs(const QString& indexs, const QString& patientUid);
    void updateChannelPlan(std::vector<ChannelPlan> channels);
    ChannelPlan getChannelPlanByIndexs(const QString& patientUid, const QString& indexs);
    ChannelPlan getChannelPlanByUid(const QString& uid);
    void insertChannelPlan(ChannelPlan channels);
    ChannelPlan getChannelPlanByDetectorType(const QString& patientUid, int detectorType);
protected:
    std::unique_ptr <CChannelPlanRepository> channelRepository ;
};

END_NX_NAMESPACE

#endif
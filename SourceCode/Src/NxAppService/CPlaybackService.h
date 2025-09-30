/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   December 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "COscillographService.h"

BEGIN_NX_NAMESPACE

class NX_APP_SERVICE_EXPORT CPlaybackService  : public COscillographService
{
    Q_OBJECT

public:
    CPlaybackService(const QString& patientUid, const QString& _recordUid, QObject *parent = nullptr);
    virtual ~CPlaybackService() override;

    // 数据库操作
    //virtual ChannelPlan getChannelPlanByUid(const QString& uid) override;
    virtual std::vector<ChannelPlan> getChannelPlans() const  override;
    //virtual std::vector<ChannelName> getChannelNamesByPlan(const QString& plansUid)  override;
    //virtual void updateChannelPlan(const ChannelPlan& plan) override;
    //virtual QPoint getChannelPositon(const QString& channelPlanUid, const QString& customName) override;
    //virtual ChannelName getChannelByChannelPosition(const int x, const int y) override;
};

END_NX_NAMESPACE
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_CHANNELPLAN_REPOSITORY_H
#define __C_CHANNELPLAN_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"

BEGIN_NX_NAMESPACE
class CChannelNameRepository;
struct ChannelPlan;

class NXDBMANAGER_EXPORT CChannelPlanRepository : public IRepository<ChannelPlan>
{
public:
    CChannelPlanRepository();
    virtual ~CChannelPlanRepository() override;

    std::vector<ChannelPlan> getChannelPlanByPatient(const QString & patientUid);

    std::vector<ChannelPlan> getByPatientAndRecordUID(const QString& patientUid, const QString& recordUid);

    void deleteChannelPlanByIndexs(const QString & indexs, const QString& patientUid);

    void updateChannelPlan(const std::vector<ChannelPlan> &channel);

    ChannelPlan getChannelPlanByIndexs(const QString& patientUid, const QString& indexs);

    ChannelPlan  getChannelPlanByUid(const QString& uid);

    void insertChannelPlan(ChannelPlan channels);

    ChannelPlan getChannelPlanByDetectorType(const QString& patientUid, int detectorType);

protected:

    std::unique_ptr <CChannelNameRepository> channelNameRepository;
   
};

END_NX_NAMESPACE

#endif

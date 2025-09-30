/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CChannelPlanRepository.h"
#include "CChannelNameRepository.h"
#include "NxEntity/ChannelPlan.h"
BEGIN_NX_NAMESPACE

CChannelPlanRepository::CChannelPlanRepository()
    :IRepository(),
    channelNameRepository(std::make_unique<CChannelNameRepository>())
{
}

CChannelPlanRepository::~CChannelPlanRepository()
{

}

std::vector<ChannelPlan> CChannelPlanRepository::getChannelPlanByPatient(const QString& patientUid)
{
    
    auto channelplan = IRepository::db->query<ChannelPlan>(" where PatientFK = '"+ patientUid.toStdString() +  "'" +
        " and IsDeleted = 0 "+
        " and ( RecordUidFK IS NULL " +
        " or RecordUidFK = '')" + 
        " order by Indexs");
    if (channelplan.empty())
    {
        return std::vector<ChannelPlan>();
    }
    else {

        return channelplan;
    }

}

std::vector<ChannelPlan> CChannelPlanRepository::getByPatientAndRecordUID(const QString& patientUid, const QString& recordUid)
{
    auto channelPlan = IRepository::db->query<ChannelPlan>(" where PatientFK = '" + patientUid.toStdString() + "'"+
        " and IsDeleted = 0 " +
        " and RecordUidFK = '" + recordUid.toStdString() + "'" +
        " order by Indexs ");
    if (channelPlan.empty())
    {
        return std::vector<ChannelPlan>();
    }
    else {

        return channelPlan;
    }
}

ChannelPlan CChannelPlanRepository::getChannelPlanByIndexs(const QString& patientUid, const QString& indexs)
{
    auto channelPlan = IRepository::db->query<ChannelPlan>(" Indexs = '" + indexs.toStdString() + "'" +
        " and PatientFK = '" + patientUid.toStdString() + "'" +
        " and IsDeleted = 0 "+
        " and ( RecordUidFK IS NULL " +
        " or RecordUidFK = '' )");
    if (channelPlan.empty())
    {
        return ChannelPlan();
    }
    else
    {
        return channelPlan[0];
    }
}

ChannelPlan CChannelPlanRepository::getChannelPlanByDetectorType(const QString& patientUid, int detectorType)
{
    auto channelPlan = IRepository::db->query<ChannelPlan>(" DetectorType = '" + QString::number(detectorType).toStdString() + "'" +
        " and PatientFK = '" + patientUid.toStdString() + "'" +
        " and IsDeleted = 0 "+
        " and ( RecordUidFK IS NULL " + 
        " or RecordUidFK = '')");
    if (channelPlan.empty())
    {
        return ChannelPlan();
    }
    else
    {
        return channelPlan[0];
    }
}

void CChannelPlanRepository::deleteChannelPlanByIndexs(const QString& indexs, const QString& patientUid)
{
    auto channelPlan = IRepository::db->query<ChannelPlan>(" Indexs = '" + indexs.toStdString()+"'" +
        " and PatientFK = '" + patientUid.toStdString() + "'" +
        " and IsDeleted = 0 "+
        " and (RecordUidFK IS NULL " +
        " or RecordUidFK = '')");
    if (channelPlan.empty())
    {
        return;
    }
    auto updataPlan = channelPlan[0];
    updataPlan.IsDeleted = true;

    IRepository::db->update(updataPlan);
    channelNameRepository->deleteChannelBychannelFK(QString::fromStdString(updataPlan.UID));
}

void CChannelPlanRepository::updateChannelPlan(const std::vector<ChannelPlan>& channels)
{
    for (auto channel: channels)
    {
        IRepository::db->update(channel);
    }
}

ChannelPlan  CChannelPlanRepository::getChannelPlanByUid(const QString& uid)
{
    auto channelplan = IRepository::db->query<ChannelPlan>(" UID = '" + uid.toStdString() + "'" + 
        " and IsDeleted = 0  "+
        " and ( RecordUidFK IS NULL " +
        " or RecordUidFK = '' )");
    if (channelplan.empty())
    {
        return ChannelPlan();
    }
    else
    {
        return channelplan[0];
    }
}
void CChannelPlanRepository::insertChannelPlan(ChannelPlan channels)
{
    IRepository::db->insert(channels);
}
END_NX_NAMESPACE



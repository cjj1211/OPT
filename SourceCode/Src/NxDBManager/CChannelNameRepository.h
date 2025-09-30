/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_CHANNELNAME_REPOSITORY_H
#define __C_CHANNELNAME_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"
#include <QPair> 
#include <QPoint>

BEGIN_NX_NAMESPACE
struct ChannelName;
class NXDBMANAGER_EXPORT CChannelNameRepository : public IRepository<ChannelName>
{
public:
    CChannelNameRepository();
    virtual ~CChannelNameRepository() override;
    QPoint getChannelPositionByUid(const QString& channelPlanUid, const QString& customName);
    QPair<int, int> getChannelPositon(const QString& fk,const QString& CustomName);
    void deleteChannelBychannelFK(const QString & fk);
    std::vector<ChannelName> getChannelNameByChannelPlanFK(const QString& fk);
    ChannelName getChannelBychannelPosition(const QString& channelPlanUid, const int x, const int y, const QString& index);
    ChannelName getChannelBychannelPosition(const int x, const int y, const QString& index, const QString& fk);

    void updateChannelName(ChannelName channelname);
    void updateChannelName(std::vector<ChannelName> channelname);
};

END_NX_NAMESPACE

#endif

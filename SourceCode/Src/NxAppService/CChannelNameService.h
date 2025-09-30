/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CHANNEL_NAME_SERVICE_H
#define __CHANNEL_NAME_SERVICE_H

#include "nxappservice_global.h"

#include "IAppService.h"

#include<QPair>
BEGIN_NX_NAMESPACE

class CChannelNameRepository;

struct ChannelName;
class NX_APP_SERVICE_EXPORT CChannelNameService
    :public IAppService
{
    Q_OBJECT
public:
    CChannelNameService(QObject* parent = nullptr);
    virtual ~CChannelNameService() override;

    QPair<int, int> getChannelPositon(const QString& fk,const QString& CustomName);
    void deleteChannelBychannelFK(const QString& fk);
    std::vector<ChannelName> getChannelBychannelFK(const QString& fk);

    void updateChannelName(std::vector<ChannelName>);
    //ChannelName getChannelBychannelPosition(const int x ,const int y, const QString& fk);
    ChannelName getChannelBychannelPosition(const int x, const int y,const QString& index, const QString& fk);
    void updateChannelName(ChannelName channelname);
   
protected:
    std::unique_ptr <CChannelNameRepository> channelNameRepository ;
};

END_NX_NAMESPACE

#endif
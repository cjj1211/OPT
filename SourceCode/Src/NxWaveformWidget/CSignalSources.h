/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_SIGNAL_SOURCES_H
#define __C_SIGNAL_SOURCES_H
#include "nxwaveformwidget_global.h"
#include "CDiscreteList.h"
#include "UiChannel.h"
#include <QStringList>
#include <QTime>
#include <QSet>

BEGIN_NX_NAMESPACE

class NX_WAVEFORM_WIDGET_EXPORT CSignalSources
{
public:
	~CSignalSources();

    static CSignalSources& instance();

    void reset();

    // 管理信号通道数据
    void addChannel(UiChannel* channel);

    void removeAllChannels();

    UiChannel* channelByName(const QString& nativeName) const;

    UiChannel* channelByIndex(int index) const;

    UiChannel* channelByUserOrder(int index) const;

    UiChannel* selectedChannel() const;

    int minUserOrder(UiSignalType type) const;

    int numChannels() const;

    int numEnabledChannels() const;

    void setChannelSelected(const std::string& nativeName, bool select) const;

    void setChannelEnabled(const QString& nativeChannelName, const bool enabled);

    QStringList getDisplayListAmplifiers() const;

private:
    CSignalSources();
    CSignalSources(const CSignalSources&) = delete;
    CSignalSources(const CSignalSources&&) = delete;
    CSignalSources& operator =(const CSignalSources&) = delete;
    CSignalSources& operator =(const CSignalSources&&) = delete;

    std::vector<UiChannel*> signalChannels;
    std::map<QString, UiChannel*> channelMap;   // map from native name to channel pointer, for quick access
};

#define signalManager CSignalSources::instance()

END_NX_NAMESPACE
#endif
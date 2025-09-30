#include "CSignalSources.h"
#include <QDebug>
#include <format>
CSignalSources::CSignalSources()

{
}

inline CSignalSources::~CSignalSources()
{
    for (const auto& signalChannel : signalChannels)
    {
        delete signalChannel;
    }
    signalChannels.clear();
}

CSignalSources& CSignalSources::instance()
{
    static CSignalSources _instance;
    return _instance;
}

void CSignalSources::reset()
{
    // TODO
    signalChannels.clear();
    channelMap.clear();
}

void CSignalSources::setChannelEnabled(const QString& nativeChannelName, const bool enabled)
{
    auto channel = channelByName(nativeChannelName); 
    if (channel->isEnabled() != enabled) {
        channel->setEnabled(enabled);
    } 
}

QStringList CSignalSources::getDisplayListAmplifiers() const
{
    QStringList displayList;
    QStringList orderedChannelList;
    for (int i = 0; i < numChannels(); ++i) {
        const UiChannel* channel = channelByUserOrder(i);
        if (channel) {
            orderedChannelList.append(channel->getNativeName());
        }
    }

    // Arrange by channel
    for (int i = 0; i < orderedChannelList.size(); ++i) {
        displayList.append(orderedChannelList[i]);
    }

    return displayList;
}

void CSignalSources::setChannelSelected(const std::string& nativeName, bool select) const
{
    UiChannel* channel = channelByName(QString::fromStdString(nativeName));
    if (!channel) {
        LOG_ERR("SignalSources::selectWaveform: channel not found from name " + nativeName);
        return;
    }
    channel->setIsSelected(select);
}


// 管理信号通道数据

void CSignalSources::addChannel(UiChannel* channel)
{
    signalChannels.emplace_back(channel);
    channelMap[channel->getNativeName()] = channel;
}

void CSignalSources::removeAllChannels()
{
    for (const auto& signalChannel : signalChannels)
    {
        delete signalChannel;
    }
    signalChannels.clear();
    channelMap.clear();
}

UiChannel* CSignalSources::channelByName(const QString& nativeName) const
{
    const auto nativeNameStr = nativeName.section('|', 0, 0);  // strip off filter name, if present
    if (const auto p = channelMap.find(nativeNameStr); p != channelMap.end()) {
        return p->second;
    }
    //    cerr << "SignalSources::channelByName: name not found: " << nativeNameStr << EndOfLine;
    return nullptr;
}

UiChannel* CSignalSources::channelByIndex(int index) const
{
    if (signalChannels.size() > index) {
        return signalChannels[index];
    }

    return nullptr;
}

inline UiChannel* CSignalSources::channelByUserOrder(int index) const
{
    UiChannel* channel = nullptr;
    for (int i = 0; i < numChannels(); ++i) {
        channel = signalChannels[i];
        if (channel->getUserOrder() == index) {
            return channel;
        }
    }
    return nullptr;
}

UiChannel* NX::CSignalSources::selectedChannel() const
{
    UiChannel* channel = nullptr;
    for (int i = 0; i < signalChannels.size(); ++i) {
        channel = signalChannels[i];
        if (channel->isSelected()) {
            return channel;
        }
    }
    return nullptr;
}

inline int CSignalSources::minUserOrder(UiSignalType type) const
{
    int min = std::numeric_limits<int>::max();
    for (int i = 0; i < (int)signalChannels.size(); ++i) {
        if (signalChannels[i]->getSignalType() == type) {
            if (signalChannels[i]->getUserOrder() < min) {
                min = signalChannels[i]->getUserOrder();
            }
        }
    }
    return min;
}

inline int CSignalSources::numChannels() const { return static_cast<int>(signalChannels.size()); }

int NX::CSignalSources::numEnabledChannels() const
{
    int enabledChannelCounts = 0;
    for (auto channel : signalChannels) {
        if (channel->isEnabled()) {
            enabledChannelCounts++;
        }
    }
    return enabledChannelCounts;
}

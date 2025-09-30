/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DISPLAY_WAVEFORM_H
#define __C_DISPLAY_WAVEFORM_H


#include "nxwaveformwidget_global.h"
#include "CSignalSources.h"
#include <QColor>

const int DividerSectionID = 99999;    // Must be larger than all other section IDs.

struct DisplayedWaveform
{
public:
    DisplayedWaveform(const QString& waveName_, WaveformType waveformType_, UiChannel* channel_) :
		waveName(waveName_),
        waveformType(waveformType_),
        channel(channel_),
        spacingAbove(3),
        spacingBelow(3),
        yCoord(-1),
        yTop(-1),
        yBottom(-1),
        yTopLimit(-1),
        yBottomLimit(-1),
        sectionID(-1),
        isCurrentlyVisible(true)
    {}

    QString waveName;
    WaveformType waveformType;
    UiChannel* channel;
    int spacingAbove;
    int spacingBelow;
    int yCoord;
    int yTop;
    int yBottom;
    int yTopLimit;
    int yBottomLimit;
    int sectionID;   // DividerId for dividers, otherwise unique number common to all waveforms in one division
    bool isCurrentlyVisible;

    bool isDivider() const { return waveformType == WaveformDivider; }
    bool isAmplifier() const { return waveformType == AmplifierWaveform; }
    bool isSelected() const { if (channel) { return channel->isSelected(); } else return false; }
    bool isEnabled() const { if (channel) { return channel->isEnabled(); } else return true; }  // Default to true for dividers.
    void setEnabled(bool enable) { if (channel) { signalManager.setChannelEnabled(channel->getNativeName(), enable); } }
    QColor getColor() const { 
        if (channel) { 
            return QColor(channel->getColor());
        } 
        else 
            return QColor(255, 255, 255); 
    }

    QString waveNameWithoutFilter() const { return waveName.section('|', 0, 0); }

    static WaveformType translateSignalTypeToWaveformType(UiSignalType signalType);
} ;

#endif // !__C_DISPLAY_WAVEFORM_H

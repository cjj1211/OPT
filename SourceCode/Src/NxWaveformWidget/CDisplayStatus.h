/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DISPLAY_STATUS
#define __C_DISPLAY_STATUS
#include "nxwaveformwidget_global.h"
#include "CDiscreteList.h"
#include <QMap>
#include <QSet>
#include <QtWidgets>

BEGIN_NX_NAMESPACE

// 显示状态
class CDisplayStatus
{
public:
    static NX_WAVEFORM_WIDGET_EXPORT CDisplayStatus& instance();

    NX_WAVEFORM_WIDGET_EXPORT void reset();
    
    NX_WAVEFORM_WIDGET_EXPORT QSet<QString> getDisplayWaveformNames() const;

    // 灵敏度
    NX_WAVEFORM_WIDGET_EXPORT float getYScale() const;
    
    NX_WAVEFORM_WIDGET_EXPORT float getYScale(int index) const;
    
    NX_WAVEFORM_WIDGET_EXPORT int getYScaleIndex() const;

    NX_WAVEFORM_WIDGET_EXPORT void shiftYScale(int index)  const;

    NX_WAVEFORM_WIDGET_EXPORT void setYScale(int index) { yScale->setIndex(index); }
    
    NX_WAVEFORM_WIDGET_EXPORT int getYScalerNumOfItems() const;

    NX_WAVEFORM_WIDGET_EXPORT std::string getYScaleName(int index) const;
    
    // 走纸速度
    NX_WAVEFORM_WIDGET_EXPORT double getPaperSpeed() const;
    
    NX_WAVEFORM_WIDGET_EXPORT void shiftPaperSpeed(int index) { tPaperSpeed->shiftIndex(index); }

    NX_WAVEFORM_WIDGET_EXPORT void setPaperSpeed(int index) { tPaperSpeed->setIndex(index); }
    
    NX_WAVEFORM_WIDGET_EXPORT int getPaperSpeedIndex()  const { return tPaperSpeed->getIndex(); }

    NX_WAVEFORM_WIDGET_EXPORT std::string getPaperSpeedName(const int index) const { return tPaperSpeed->items[index].displayedValueName; }
    
    NX_WAVEFORM_WIDGET_EXPORT int getPaperSpeedNumberOfItems()  const { return tPaperSpeed->numberOfItems(); }

    NX_WAVEFORM_WIDGET_EXPORT double getDispWaveAreaLength() const { return dispWaveAreaLength; }

    void setDispWaveAreaLength(const double len) { dispWaveAreaLength = len; }

    // sweep模式为绘制区域波形的最右侧像素x坐标， roll模式为波形最左侧位置
    NX_WAVEFORM_WIDGET_EXPORT int getValidDataIndex()  const;; // 绘制区域的最右侧像素x坐标

    NX_WAVEFORM_WIDGET_EXPORT void setValidDataIndex(const int index) { validDataIndex = index; };

    NX_WAVEFORM_WIDGET_EXPORT double getZoneLength() const { return dispWaveAreaLength / numRefreshZones; }

    NX_WAVEFORM_WIDGET_EXPORT int getRefreshZoneNum() const { return numRefreshZones; }

    NX_WAVEFORM_WIDGET_EXPORT double getTimespanPerScreen() { return timespanPerScreen; }

    void setTimespanPerScreen(double timespan) { 
        timespanPerScreen = timespan;
        numRefreshZones = timespan * 20; // 每秒分10段进行传输
        if (numRefreshZones < 1)
        {
            numRefreshZones = 1;
        }
    }

    // 获取显示状态
    QMap<int, QString> getTimeAxisLabels() { return timeAxisMap; }

    void clearTimeAxisLabels() {  QMap<int, QString>().swap(timeAxisMap); }

    void insertTimeAxisLabel(int index, QString val) { timeAxisMap.insert(index, val); }

    QString getBackgroundColor() { return backgroundColor; }

    QString getTimelineBackground() { return timelineBackground; }

    bool isSweepFirstTime() { return sweepFirstTime; }

    void setSweepFirstTime(bool isSweepFirstTime) { sweepFirstTime = isSweepFirstTime; }

    bool isShowRuler() { return showRuler; }

    void setShowRuller(bool isShow) { showRuler = isShow; }

    bool isRollMode() { return rollMode; }

    void setRollMode(const bool isRoll) { rollMode = isRoll; }

    bool isClipWaveforms() { return clipWaveforms; }

    bool isShowDisabledChannels() { return showDisabledChannels; }

    bool isShowVerticalTimeLines() { return showVerticalTimeLines; }

    void updateDisplayWaveformNames(const QStringList& pinnedWaves, const QStringList& displayWaves);

    
private:
    CDisplayStatus();
    CDisplayStatus(const CDisplayStatus&) = delete;
    CDisplayStatus(const CDisplayStatus&&) = delete;
    CDisplayStatus& operator =(const CDisplayStatus&) = delete;
    CDisplayStatus& operator =(const CDisplayStatus&&) = delete;

    // 信号状态
    CDiscreteList* tPaperSpeed;
    CDiscreteList* yScale;

    QMap<int, QString> timeAxisMap;

    QString backgroundColor{ "#000000" };
    QString timelineBackground{ "#D5D5D5" };

    int validDataIndex;// 已绘制区域数量的计数
    double dispWaveAreaLength;
    double numRefreshZones; // 一屏数据少于0.2s时，numRefreshZones = 1; 
    double timespanPerScreen; 

    bool sweepFirstTime;
    bool showRuler;
    bool rollMode;
    bool clipWaveforms;
    bool showDisabledChannels;
    bool showVerticalTimeLines;

    QSet<QString> visibleWaveNames;
};

#define displayStatus CDisplayStatus::instance()

END_NX_NAMESPACE

#endif

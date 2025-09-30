#ifndef __C_WAVEFORM_DISPLAY_MANAGER_H
#define __C_WAVEFORM_DISPLAY_MANAGER_H
#include "nxwaveformwidget_global.h"
#include "CSignalSources.h"
#include "CDisplayStatus.h"
#include "CMinMax.h"

#include <QLine>
#include <QPainter>
#include <map>
#include <string>
#include <mutex>
#include <vector>

BEGIN_NX_NAMESPACE

struct WaveformSegment;
class UiDisplayDto;
const float ScaleFactorY = 1.0F;

struct WaveformDisplayDataStore
{
public:
    WaveformDisplayDataStore() :
        yScaleType(UnknownYScale),
        hasAlreadyLoaded(false),
        isOutOfDate(false)
    {}

    // Internal state
    YScaleType yScaleType;
    bool hasAlreadyLoaded;
    bool isOutOfDate;

    void calculateParameters(const double sampleRate)
    {
        auto dispWidth = displayStatus.getDispWaveAreaLength();
        auto timespan = displayStatus.getTimespanPerScreen();
        auto zoneNum = displayStatus.getRefreshZoneNum();
        pixelsPerSample = dispWidth / (timespan * sampleRate) ;
        useVerticalLines = pixelsPerSample < 1.0F;

        if (useVerticalLines) {
            yMin.resize(dispWidth);
            yMax.resize(dispWidth);
            verticalLines.resize(dispWidth);
            yData.clear();
            points.clear();
        }
        else {
            double sampleCntPerScreen = timespan * sampleRate + 1;

            yData.resize(sampleCntPerScreen);
            points.resize(sampleCntPerScreen);
            yMin.clear();
            yMax.clear();
            verticalLines.clear();
        }
        isOutOfDate = false;
    }

    bool useVerticalLines;
    double pixelsPerSample;

    // Data stores: raw y coordinates in sequence
    std::vector<float> yMin;
    std::vector<float> yMax;
    std::vector<float> yData;

    // Data stores: screen coordinates
    std::vector<QLineF> verticalLines;
    std::vector<QPointF> points;
};


class WaveformDisplayManager
{
public:
    WaveformDisplayManager( int maxWidthInPixels_);
    ~WaveformDisplayManager();
    int playbackStatus ;
    bool addWaveform(const QString& waveName);  // Return false if waveName is already in waveform list.
    bool removeWaveform(const QString& waveName);  // Returns false if waveName is not in waveform list.
    void removeAllWaveforms();
    void setMaxWidthInPixels(int maxWidthInPixels_);
    void setTPaperSpeed();
    void resetAll();
    void loadNewData(UiDisplayDto& uiDisplayDto, const QString& waveName);

    // 获取每个像素多对应的电压值，单位uv
    float getYScaleFactor(YScaleType yScaleType) const;
    float getYScaleFactor(const QString& waveName) const;

    double getPixelPerMM();

    void draw(QPainter &painter, const QString& waveName, QPoint position, QColor color);
    void drawWaveformInRollMode(QPainter& painter, const QString& waveName, QPoint position, QColor color, WaveformDisplayDataStore* ds);
    void drawWaveformInSweepMode(QPainter& painter, const QString& waveName, QPoint position, QColor color, WaveformDisplayDataStore* ds);
    void drawDivider(QPainter &painter, int yPos, int xStart, int xEnd);

private:
    // Input parameters
    int maxWidthInPixels;
    //int numRefreshZones;
    int timeSpan; // 1屏展示的数据长度

    // Calculated display data parameters
    //int zoneWidthInPixels;
    // 整个波形显示区域像素宽度

    // Waveform data mapped to waveform name
    std::map<QString, WaveformDisplayDataStore*> waveformDisplayData;
    
    const QColor ComplianceLimitColor = QColor(255, 0, 0);
    const QColor AmpSettleColor = QColor(255, 255, 215);
    const QColor ChargeRecovColor = QColor(215, 255, 215);

    void calculateParameters();
    void loadDataSegment(UiDisplayDto& uiDisplayDto, const QString& waveName,  WaveformDisplayDataStore* ds);

};

END_NX_NAMESPACE
#endif

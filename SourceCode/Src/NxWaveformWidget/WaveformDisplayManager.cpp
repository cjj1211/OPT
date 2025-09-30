
#include "waveformdisplaymanager.h"
#include "CommunicationDto.h"
#include "CMinMax.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QTime>
#include <QColor>
#include <QDebug>

#include <iostream>

BEGIN_NX_NAMESPACE
constexpr float epsilonX = 0.49F;    // Help to resolve plotting issue where short line segments were invisible.
WaveformDisplayManager::WaveformDisplayManager( int maxWidthInPixels_) :
    maxWidthInPixels(maxWidthInPixels_)
{
    LOG("Beginning of WaveformDisplayManager ctor");
    
    displayStatus.setValidDataIndex(0);

    LOG("About to call calculateParameters()");
    calculateParameters();
    LOG("Completed calculateParameters(). End of ctor");
}

WaveformDisplayManager::~WaveformDisplayManager()
{
    if (!waveformDisplayData.empty()) {
        std::map<QString, WaveformDisplayDataStore*>::const_iterator it = waveformDisplayData.begin();
        while (it != waveformDisplayData.end()) {
            delete it->second;
            ++it;
        }
    }
}

void WaveformDisplayManager::calculateParameters()
{
    LOG("Beginning of calculateParameters()");
    
    // 每厘米宽度对应的像素数
    const auto pixelsPerMM = getPixelPerMM(); 
    // mm/s
    const int speed = displayStatus.getPaperSpeed(); 
    // 调整绘制宽度为走纸速度的整数倍
    auto pixelsPerSec = speed * pixelsPerMM;
    int timespan = floor(maxWidthInPixels / pixelsPerSec);
    //if (timespan == 0) // 波形只显示走纸速度整数倍
    if (true) 
    {
        displayStatus.setDispWaveAreaLength(maxWidthInPixels - 20);
        displayStatus.setTimespanPerScreen((double(maxWidthInPixels - 20)/ pixelsPerSec)); // 每秒分10段进行传输
    }
    else {
        auto width = timespan * pixelsPerSec; // 确保物理尺寸长度准确
        displayStatus.setDispWaveAreaLength(width);
        displayStatus.setTimespanPerScreen(timespan); // 每秒分10段进行传输
    }
    
    resetAll();
    LOG_INFO("Completed resetAll(). End of calculateParameters()");
    
//    cout << EndOfLine;
//    cout << "WaveformDisplayManager::calculateParameters:" << EndOfLine;
//    cout << "   zone width in pixels = " << zoneWidthInPixels << EndOfLine;
//    cout << "   width in pixels = " << widthInPixels << EndOfLine;
//    cout << "   samples per zone = " << samplesPerZone << EndOfLine;
//    cout << "   pixels per sample = " << pixelsPerSample << EndOfLine;
//    cout << "   use vertical lines = " << useVerticalLines << EndOfLine;
//    cout << "   length = " << length << EndOfLine;
//    cout << EndOfLine;
}

bool WaveformDisplayManager::addWaveform(const QString& waveName)
{
    if (waveformDisplayData.find(waveName) != waveformDisplayData.end()) return false;  // No repeats!  Do not read from the Waveform FIFO twice!

    WaveformDisplayDataStore* ds = new WaveformDisplayDataStore;
    ds->yScaleType = WidebandYScale;

    waveformDisplayData[waveName] = ds;
    auto channel = signalManager.channelByName(waveName);
    ds->calculateParameters(channel->getSampleRate());
    return true;
}

bool WaveformDisplayManager::removeWaveform(const QString& waveName)
{
    if (waveformDisplayData.find(waveName) == waveformDisplayData.end()) return false;
    delete waveformDisplayData.at(waveName);
    waveformDisplayData.erase(waveName);
    return true;
}

void WaveformDisplayManager::removeAllWaveforms()
{
    waveformDisplayData.clear();
}

void WaveformDisplayManager::setTPaperSpeed()
{
    calculateParameters();
}

void WaveformDisplayManager::setMaxWidthInPixels(int maxWidthInPixels_)
{
    if(maxWidthInPixels != maxWidthInPixels_)
    {
        maxWidthInPixels = maxWidthInPixels_;
        calculateParameters();
    }
}

void WaveformDisplayManager::loadNewData(UiDisplayDto& uiDisplayDto, const QString& waveName)
{
	const std::map<QString, WaveformDisplayDataStore*>::const_iterator it = waveformDisplayData.find(waveName);
    if (it == waveformDisplayData.end()) {
        std::cout << "WaveformDisplayManager::loadNewData: Could not find waveName " << waveName.toStdString() << '\n';
        return;
    }
    WaveformDisplayDataStore* ds = it->second;
    if (!ds) return;

    loadDataSegment(uiDisplayDto, waveName, ds);
}

void WaveformDisplayManager::loadDataSegment(UiDisplayDto& uiDisplayDto, const QString& waveName, WaveformDisplayDataStore* ds)
{
    auto channel = signalManager.channelByName(waveName);
    auto paperSpeed = displayStatus.getPaperSpeed(); // etc. 15mm/s
    auto pixelsPerSec = getPixelPerMM(); // 5 pixels / mm
    auto sampleRate = channel->getSampleRate(); // etc. 1000Hz / s
    auto samplesPerPixel =  1.0 / ds->pixelsPerSample;
    SignalSegment* signalSegment = uiDisplayDto.getSignalSegment(waveName);
    if (uiDisplayDto.isContainsWave(waveName))
    {
        if (ds->useVerticalLines)
        {
            // 通过滑动窗口获取窗口内最大值和最小值，所以读取数据时候要多读(samplesPerPixel - 1)位
            const auto toSize = sizeof(float) * std::min(ds->yMin.size(), signalSegment->yMinData.size());
            memcpy(ds->yMin.data(), signalSegment->yMinData.data(), toSize);
            memcpy(ds->yMax.data(), signalSegment->yMaxData.data(), toSize);
        }
        else
        {
            const auto toSize = sizeof(float) * std::min(ds->yData.size(), signalSegment->yData.size());
            memcpy(ds->yData.data(), signalSegment->yData.data(), toSize);
        }
    }
}

// returns pixel/uv
float WaveformDisplayManager::getYScaleFactor(const QString& waveName) const
{
    auto it = waveformDisplayData.find(waveName);
    if (it == waveformDisplayData.end()) return 0.0F;
    WaveformDisplayDataStore* ds = it->second;
    if (!ds) return 0.0F;
    return getYScaleFactor(ds->yScaleType);
}

double WaveformDisplayManager::getPixelPerMM()
{
    const auto physicalSize = qApp->primaryScreen()->physicalSize(); // 屏幕物理尺寸 单位 mm
    const auto size = qApp->primaryScreen()->size();
    const auto pixelsPerMm = size.height() / physicalSize.height(); // 每mm宽度对应的像素数
    return pixelsPerMm;
}

// returns pixel/uv
float WaveformDisplayManager::getYScaleFactor(YScaleType yScaleType) const
{
    float yScale = static_cast<float>(displayStatus.getYScale());
    const auto physicalSize = qApp->primaryScreen()->physicalSize(); // 屏幕物理尺寸 单位 mm
    const auto size = qApp->primaryScreen()->size();
    const auto pixelsPerMm = size.height() / physicalSize.height(); // 每mm宽度对应的像素数
    return pixelsPerMm / yScale ;
}

void WaveformDisplayManager::draw(QPainter &painter, const QString& waveName, QPoint position, QColor color)
{
    auto it = waveformDisplayData.find(waveName);
    if (it == waveformDisplayData.end()) return;
    WaveformDisplayDataStore* ds = it->second;
    if (!ds) return;
    if (ds->isOutOfDate) return;

    if (!displayStatus.isRollMode()) {  // Sweep mode (or rewinding/fast forwarding)
        drawWaveformInSweepMode(painter, waveName, position,color, ds);
    } else {  // Roll mode
        drawWaveformInRollMode(painter, waveName, position, color, ds);
    }
}

void WaveformDisplayManager::drawWaveformInRollMode(QPainter& painter, const QString& waveName, QPoint position, QColor color, WaveformDisplayDataStore* ds)
{
    const int length = displayStatus.getDispWaveAreaLength();
    if (displayStatus.getValidDataIndex() >= length) return;
    const float yScaleFactor = -getYScaleFactor(ds->yScaleType);
    auto yOffset = static_cast<float>(position.y());
    auto x = static_cast<float>(displayStatus.getValidDataIndex() + position.x());
    if (ds->useVerticalLines) {
        for (int i = displayStatus.getValidDataIndex(); i < length; ++i) {
            ds->verticalLines[i] = QLineF(x, yScaleFactor * ds->yMin[i] + yOffset,
                x + epsilonX, yScaleFactor * ds->yMax[i] + yOffset);
            ++x;
        }
        // Draw main waveform.
        painter.setPen(color);
        painter.drawLines(&ds->verticalLines[displayStatus.getValidDataIndex()], length - displayStatus.getValidDataIndex());
    }
    else {
        for (int i = displayStatus.getValidDataIndex(); i < length; ++i) {
            ds->points[i] = QPointF(x, yScaleFactor * ds->yData[i] + yOffset);
            x += ds->pixelsPerSample;
        }
        // Draw main waveform.
        painter.setPen(color);
        painter.drawPolyline(&ds->points[displayStatus.getValidDataIndex()], length - displayStatus.getValidDataIndex());
    }
}

void WaveformDisplayManager::drawWaveformInSweepMode(QPainter& painter, const QString& waveName, QPoint position, QColor color, WaveformDisplayDataStore* ds)
{
    const float yScaleFactor = -getYScaleFactor(ds->yScaleType);

    
    int validDataIndex = displayStatus.getValidDataIndex();
    auto yOffset = static_cast<float>(position.y());
    auto x = static_cast<float>(position.x());
    if (ds->useVerticalLines) {
        int dispWidth = displayStatus.getDispWaveAreaLength();
        for (int i = 0; i < dispWidth; ++i) {
            // x 为水平方向
            // y 为竖直方向，与幅值相关
            ds->verticalLines[i] = QLineF(x, yScaleFactor * ds->yMin[i] + yOffset,
                x + epsilonX, yScaleFactor * ds->yMax[i] + yOffset);
            ++x;
        }
        // Draw main waveform.
        painter.setPen(color);
        painter.drawLines((ds->verticalLines).data(), validDataIndex);
        if (!displayStatus.isSweepFirstTime() && validDataIndex < dispWidth) {
            painter.drawLines(&ds->verticalLines[validDataIndex], dispWidth - validDataIndex);
        }
    }
    else {
        double dispWidth = displayStatus.getDispWaveAreaLength();
        int endPosition = round(((double)validDataIndex) / dispWidth * ds->yData.size());
        auto channel = signalManager.channelByName(waveName);

        for (int i = 0; i < ds->yData.size(); ++i) {
            ds->points[i] = QPointF(x, yScaleFactor * ds->yData[i] + yOffset);
            x += ds->pixelsPerSample;
            //if (i + 1 >= endPosition) {
            //    break;
            //}
        }
        // Draw main waveform.
        painter.setPen(color);
        
        painter.drawPolyline((ds->points).data(), std::min(endPosition, (int)ds->points.size()));

        if (!displayStatus.isSweepFirstTime() && (endPosition < ds->points.size())) {
            painter.drawPolyline(&ds->points[endPosition], static_cast<int>(ds->points.size()) - endPosition);
        }
    }
}

// 绘制分隔符
void WaveformDisplayManager::drawDivider(QPainter &painter, int yPos, int xStart, int xEnd)
{
    painter.fillRect(QRect(xStart, yPos - 1, xEnd - xStart, 3), QBrush(Qt::darkGray));
}

void WaveformDisplayManager::resetAll()
{
    if (waveformDisplayData.empty()) return;
    displayStatus.setSweepFirstTime(true);
    LOG("Reset all display parameter.");
    displayStatus.setValidDataIndex(0);

    for (const auto& pair : waveformDisplayData) {
        auto& wavename = pair.first;
        auto ds = pair.second;
        auto channel = signalManager.channelByName(wavename);
        ds->calculateParameters(channel->getSampleRate());
    }

    LOG("Reset all display parameter done.");
}

END_NX_NAMESPACE
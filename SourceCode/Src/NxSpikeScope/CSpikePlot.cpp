#include "CSpikePlot.h"

#include <qmath.h>
#include <QMouseEvent>
#include <QPainter>
#include <QStylePainter>

#include "CPlotDecorator.h"

const QString MuSymbol = QString((QChar)0x03bc);
const QString MicroVoltsSymbol = MuSymbol + QString("V");
CSpikePlot::CSpikePlot(const QString& channelName_, const int sampleRate_, QWidget*parent)
    : QWidget(parent)
{
    setMouseTracking(true);// 设置鼠标跟踪，不然只会在鼠标按下时才会触发鼠标移动事件
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);

    samplesPreDetect = 90;
    samplesPostDetect = 180;
    slotSetWaveformName(channelName_, sampleRate_);
    slotUpdateTimeScale(2);
}

CSpikePlot::~CSpikePlot() = default;

void CSpikePlot::slotUpdateTimeScale(const int timeScale_)
{
    timeScale = timeScale_;
    samplesPreDetect = static_cast<int>(timeScale * sampleRate / 1000.0) / 2;
    samplesPostDetect = static_cast<int>(timeScale * sampleRate / 1000.0);
    preSamples.resize(samplesPreDetect + samplesPostDetect);
    preSamples.fill(0.0);
    spikeSegments.clear();
    snapshotSpikeSegments.clear();
    tStepMsec = 1000.0 / static_cast<double>(sampleRate);
    update();
}

void CSpikePlot::slotSetWaveformName(const QString& channelName_, const int sampleRate_)
{
    wavename = channelName_;
    sampleRate = sampleRate_;
    update();
}

void CSpikePlot::slotSetMaxSpikeCount(const int count)
{
    showSpikeCounts = count;
    update();
}

void CSpikePlot::slotTakeSnapshot()
{
    snapshotSpikeSegments = spikeSegments;
    spikeSegments.clear();
    update();
}

void CSpikePlot::slotUpdateWaveforms(QVector<float> waveformSegment)
{
    const auto windowWidth = preSamples.count();
    const QVector<float> data = preSamples + waveformSegment;

    bool preIsSurpassed = false;

    for(int i = windowWidth / 3; i < data.count() - windowWidth * 2 / 3; ++i)
    {
        bool surpassed;
        bool isDetected = false;
        if(threshold >= 0)
        {
            if(data[i] > threshold)
            {
                surpassed = true;
            }
            else
            {
                surpassed = false;
            }

            if (surpassed && !preIsSurpassed) isDetected = true;
        }
        else
        {
            if(data[i] < threshold)
            {
                surpassed = true;
            }
            else
            {
                surpassed = false;
            }
            
            if (surpassed && !preIsSurpassed) isDetected = true;
        }

        if(isDetected)
        {
            QVector<float> spikeSegment(windowWidth);
            for(int j = 0; j < windowWidth; ++j)
            {
                spikeSegment[j] = data[i - windowWidth / 3 + j];
            }

            spikeSegments.enqueue(spikeSegment);
            if(spikeSegments.count() > showSpikeCounts)
            {
                spikeSegments.dequeue();
            }

            i += windowWidth;
        }
        preIsSurpassed = surpassed;
    }

    const int segmentSize = waveformSegment.count();
    if(segmentSize > windowWidth)
    {
        for (int i = 0; i < windowWidth; ++i)
        {
            preSamples[i] = waveformSegment[segmentSize - windowWidth + i];
        }
    }
    else
    {
        for(int i = 0; i < segmentSize; ++i)
        {
            preSamples[windowWidth - segmentSize + i] = waveformSegment[i];
        }
    }

    // Calculate RMS level from recent waveform data.
    for(int i = 0 ; i < waveformSegment.count(); ++i)
    {
        rmsBuffer.enqueue(waveformSegment[i]);
        if(rmsBuffer.count() > sampleRate)
        {
            rmsBuffer.dequeue();
        }
    }

    latestRmsCalculation = 0.0;
    latestSpikeRateCalculation = 0;

    double sumOfSquares = 0.0;
    for (const auto sample : rmsBuffer) {
        sumOfSquares += sample * sample;
    }
    latestRmsCalculation = sqrt(sumOfSquares / static_cast<double>(rmsBuffer.count()));
    update();
}

void CSpikePlot::slotClearSpikes()
{
    spikeSegments.clear();
    update();
}

void CSpikePlot::slotClearSnapshot()
{
    snapshotSpikeSegments.clear();
    update();
}

void CSpikePlot::slotUpdateThreshold(int threshold_)
{
    if (threshold_ != threshold)
    {
        updateCoordinateTranslator();
        threshold = threshold_;
        update();
    }
}

void CSpikePlot::paintEvent(QPaintEvent* event)
{
    QPainter painter(&image);

    // Clear old display.
    const QRect imageFrame(rect());
    painter.fillRect(imageFrame, QBrush(Qt::black));

    // Draw border around Widget display area.
    painter.setPen(Qt::darkGray);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));

    const int TextBoxWidth = fontMetrics().horizontalAdvance("+5000 " + MicroVoltsSymbol);
    const int TextBoxHeight = fontMetrics().height();
    scopeFrame = imageFrame.adjusted(TextBoxWidth + 5, TextBoxHeight + 10, -8, -TextBoxHeight - 10);
    updateCoordinateTranslator();
    painter.drawRect(ct.borderRect());    // Draw borders of scope.
    //    painter.setClipRect(ct.frame()); // Allow plotting on top of borders.
    painter.setClipRect(ct.clippingRect()); // Keep plotting just inside borders.

    const CPlotDecorator plotDecorator(painter);

    // Vector for waveform plot points
    const int snippetLength = samplesPreDetect + samplesPostDetect;
    const auto polyline = new QPointF[snippetLength];

    painter.setPen(QColor(140,83,25));
    int length = snapshotSpikeSegments.count();
    for (int i = 0; i < length; ++i) {
        double time = -samplesPreDetect * tStepMsec;
        for (int t = 0; t < snippetLength; ++t) {
            const int x = ct.screenXFromRealX(time);
            polyline[t] = QPointF(ct.screenXFromRealX(time), ct.screenYFromRealY(snapshotSpikeSegments[i][t]));
            time += tStepMsec;
        }
        painter.drawPolyline(polyline, snippetLength);
    }

    // Draw current spike waveforms.
    length = spikeSegments.count();
    for (int i = 0; i < length; ++i) {
        double time = -samplesPreDetect * tStepMsec;
        for (int t = 0; t < snippetLength; ++t) {
            const int x = ct.screenXFromRealX(time);
            polyline[t] = QPointF(x, ct.screenYFromRealY(spikeSegments[i][t]));
            time += tStepMsec;
        }
        const int value = 255 - static_cast<int>(200.0 * static_cast<double>(length - i - 1) / static_cast<double>(length));
        painter.setPen(QColor(value, value, value));
        painter.drawPolyline(polyline, snippetLength);
    }

    delete[] polyline;

    painter.setClipping(false);

    painter.setPen(Qt::white);
    plotDecorator.writeLabel(wavename, ct.xLeft() + 2, ct.yTop() - 1,
        Qt::AlignLeft | Qt::AlignBottom);

    // Draw horizonal axis lines.
    painter.setPen(Qt::white);
    plotDecorator.drawHorizontalAxisLine(ct, 0.0);

    // Write voltage axis labels.
    painter.setPen(Qt::white);
    plotDecorator.drawLabeledTickMarkLeft(0, ct, 0.0, 0);
    plotDecorator.drawLabeledTickMarkLeft("+" + QString::number(ct.yMaxReal()) + " " + MicroVoltsSymbol, ct, ct.yMaxReal(), 0);
    plotDecorator.drawLabeledTickMarkLeft(QString::number(ct.yMinReal()) + " " + MicroVoltsSymbol, ct, ct.yMinReal(), 0);

    // Draw vertical axis lines.
    const int tMax = qFloor(ct.xMaxReal());
    for (int t = qCeil(ct.xMinReal()); t <= tMax; ++t) {
        if (t == 0) {
            painter.setPen(Qt::white);
        }
        else {
            painter.setPen(Qt::darkGray);
        }
        plotDecorator.drawVerticalAxisLine(ct, t);

        // Write time axis labels.
        painter.setPen(Qt::white);
        plotDecorator.drawLabeledTickMarkBottom(QString::number(t) + ((t == tMax) ? " ms" : ""), ct, t, 0, t == tMax);
    }

    // Draw horizonal threshold line.
    painter.setPen(Qt::red);
    plotDecorator.drawHorizontalAxisLine(ct, threshold);
    plotDecorator.drawLabeledTickMarkLeft(threshold, ct, threshold, 0);

    // Write RMS value to display.
    constexpr int textBoxWidth = 180;
    const int textBoxHeight = painter.fontMetrics().height();
    QString rmsText = "RMS: " + QString::number(latestRmsCalculation, 'f', (latestRmsCalculation < 9.95) ? 1 : 0) +
        " " + MicroVoltsSymbol;
    //if (latestSpikeRateCalculation > 0) {
    //    rmsText += "  " + QString::number(latestSpikeRateCalculation);
    //    if (latestSpikeRateCalculation == 1) {
    //        rmsText += " spike/s";
    //    }
    //    else {
    //        rmsText += " spikes/s";
    //    }
    //}
    painter.setPen(Qt::darkGreen);
    painter.drawText(scopeFrame.left() + 6, scopeFrame.top() + 5, textBoxWidth, textBoxHeight,
        Qt::AlignLeft | Qt::AlignTop, rmsText);

    QStylePainter stylePainter(this);
    stylePainter.drawImage(0, 0, image);
}

void CSpikePlot::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
}

void CSpikePlot::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (scopeFrame.contains(event->pos())) {
            updateCoordinateTranslator();
            threshold = qRound(ct.realYFromScreenY(event->pos().y()));
            emit signalThresholdChanged(threshold);
            update();
        }
        else {
            QWidget::mousePressEvent(event);
        }
    }
    else {
        QWidget::mousePressEvent(event);
    }
}

void CSpikePlot::wheelEvent(QWheelEvent* event)
{
    QWidget::wheelEvent(event);
}

void CSpikePlot::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
}

void CSpikePlot::resizeEvent(QResizeEvent* event)
{
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    update();
}

void CSpikePlot::updateCoordinateTranslator()
{
    double tMax = timeScale;
    double tMin = -tMax / 2.0;
    double vScale = yScale;
    ct.set(scopeFrame, tMin, tMax, -vScale, vScale);
}

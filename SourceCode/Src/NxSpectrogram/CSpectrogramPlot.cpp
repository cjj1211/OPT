#include "CSpectrogramPlot.h"
#include "CColorScale.h"
#include "CCoordinateTranslator1D.h"
#include "CPlotDecorator.h"
#include "CFastFourierTransform.h"
#include <qmath.h>
#include <QStylePainter>

BEGIN_NX_NAMESPACE

const QString MuSymbol = QString((QChar)0x03bc);
const QString MicroVoltsSymbol = MuSymbol + QString("V");
const QString SqrtSymbol = QString((QChar)0x221a);
const int maxFftSize = 16384;
const int TickMarkLength = 5;
const int TickMarkMinorLength = 3;

// log10(2) through log10(9) for logarithmic axes in plots
const double Log10_2 = 0.301029995664;
const double Log10_3 = 0.477121254720;
const double Log10_4 = 0.602059991328;
const double Log10_5 = 0.698970004336;
const double Log10_6 = 0.778151250384;
const double Log10_7 = 0.845098040014;
const double Log10_8 = 0.903089986992;
const double Log10_9 = 0.954242509439;

CSpectrogramPlot::CSpectrogramPlot(QWidget* parent)
    : QWidget(parent)
    , fftEngine(nullptr)
    , background(Qt::black)
{
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);  // If we don't do this, mouseMoveEvent() is active only when mouse button is pressed.

    psdScaleMin = -1.7;
    psdScaleMax = 3.5;
    colorScale = new CColorScale(psdScaleMin, psdScaleMax);
    psdUnitsMicro = " " + MicroVoltsSymbol + "/" + SqrtSymbol + "Hz";
    lastMouseWasInFrame = false;

    fftInputBuffer = new float[maxFftSize];

    fftEngine = new CFastFourierTransform(displayParam.sampleRate);
    setNewFftSize((int)displayParam.fftSize);
    setNewTimeScale(displayParam.tScale);
    resetSpectrogram();

    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
}

CSpectrogramPlot::~CSpectrogramPlot()
{
    delete[] fftInputBuffer;
    delete fftEngine; 
    delete colorScale;
}

QString CSpectrogramPlot::getCustomWaveName()
{
    return customWaveName;
}

void CSpectrogramPlot::setWaveform(const QString& customWaveName_, const QString& _comments, double _sampleRate)
{
    if (customWaveName == customWaveName_) return;

    customWaveName = customWaveName_;
    comments = _comments;
    displayParam.sampleRate = _sampleRate;
    if (fftEngine != nullptr)
    {
        delete fftEngine;
        fftEngine = nullptr;
    }
    fftEngine = new CFastFourierTransform(_sampleRate);
    setNewFftSize(fftSize); 
    setNewTimeScale(displayParam.tScale);
    resetSpectrogram();
    update();
}

bool CSpectrogramPlot::updateWaveforms(int numSamples, const QVector<float>& data, const QVector<uint64_t>& timestamps)
{
    //fftSize
    for (int t = 0; t < numSamples; ++t) {
        auto amplifierVal = data[t];// waveformFifo->getGpuAmplifierData(WaveformFifo::ReaderDisplay, waveformAddress, t);
        amplifierWaveformQueue.push_back(amplifierVal);
        amplifierWaveformRecordQueue.push_back(amplifierVal);
        waveformTimeStampQueue.push_back(timestamps[t]);
        sampleCounter++;
    }

    float* fftOut;
    while ((int)amplifierWaveformQueue.size() >= fftSize) {
        for (int i = 0; i < fftSize; ++i) {
            fftInputBuffer[i] = amplifierWaveformQueue[i];  // Copy N samples for FFT.
        }

        for (int i = 0; i < fftSize / 2; ++i) {    // Advance window by N/2 samples.
            amplifierWaveformQueue.pop_front();
        }
        if (spectrogramFull) {
            for (int i = 0; i < fftSize / 2; ++i) {
                amplifierWaveformRecordQueue.pop_front();
                waveformTimeStampQueue.pop_front();
            }
        }

        fftOut = fftEngine->logSqrtPowerSpectralDensity(fftInputBuffer);  // Calculate FFT and PSD.

        int fSize = (int)frequencyScale.size();
        for (int fIndex = 0; fIndex < fSize; ++fIndex) {
            psdSpectrum[fIndex] = fftOut[fIndex];
            psdSpectrogram[tIndex][fIndex] = fftOut[fIndex];   // Read out results of power spectral density (PSD).
        }

        QPainter psdPainter(&psdRawImage);
        psdPainter.drawImage(QRect(0, 0, tSize - 1, fSize), psdRawImage, QRect(1, 0, tSize - 1, fSize));  // shift existing PSD
        for (int fIndex = 0; fIndex < fSize; ++fIndex) {
            // Draw new PSD column.
            psdRawImage.setPixelColor(tSize - 1, fSize - fIndex - 1, colorScale->getColor(psdSpectrogram[tIndex][fIndex]));
        }

        if (++tIndex == tSize) {
            tIndex = 0;
            spectrogramFull = true;
        }
        if (++numValidTStepsInSpectrogram > tSize) numValidTStepsInSpectrogram = tSize;
    }

    update();
    return true;
}

QSize CSpectrogramPlot::minimumSizeHint() const
{
    return QSize(SPECPLOT_X_SIZE, SPECPLOT_Y_SIZE);
}

QSize CSpectrogramPlot::sizeHint() const
{
    return QSize(SPECPLOT_X_SIZE, SPECPLOT_Y_SIZE);
}

void CSpectrogramPlot::setDisplayMode(const SpectrogramMode _mode)
{
    displayParam.mode = _mode;
    updateFromState();
}

void CSpectrogramPlot::setFftSize(const double _fftSize) {
    displayParam.fftSize = _fftSize;
    updateFromState();
}

void CSpectrogramPlot::setFreqMin(const double _freqMin) {
    displayParam.minFreq = _freqMin;
    updateFromState();
}

void CSpectrogramPlot::setFreqMax(const double _freqMax) {
    displayParam.maxFreq = _freqMax;
    updateFromState();
}

void CSpectrogramPlot::setFreqMarkerValue(const double _markerValue) {
    displayParam.freqMarkerValue = _markerValue;
    updateFromState();
}

void CSpectrogramPlot::setShowMarker(const bool _enable) {
    displayParam.showFreqMarker = _enable;
    updateFromState();
}

void CSpectrogramPlot::setNumHarmonics(const int num) {
    displayParam.harmonicsNum = num;
    updateFromState();
}

void CSpectrogramPlot::setTScale(const int scale) {
    displayParam.tScale = scale;
    updateFromState();
}

void CSpectrogramPlot::setBackground(const QColor color)
{
    background = color;
}

void CSpectrogramPlot::updateFromState()
{
    bool spectrogramSizeChanged = false;

    if (fftSize != (int)displayParam.fftSize) {
        setNewFftSize((int)displayParam.fftSize);
        setNewTimeScale(displayParam.tScale);
        spectrogramSizeChanged = true;
    }
    else if (tScale != displayParam.tScale) {
        setNewTimeScale(displayParam.tScale);
        spectrogramSizeChanged = true;
    }

    if (fMinIndex != displayParam.minFreq || fMaxIndex != displayParam.maxFreq) {
        updateFMinMaxIndex();
    }

    if (spectrogramSizeChanged) {
        resetSpectrogram();
    }

    update();
}

void CSpectrogramPlot::setNewFftSize(int fftSize_)
{
    fftSize = fftSize_;
    fftEngine->setLength(fftSize);
    int fSize = fftSize / 2 + 1;
    fMinIndex = 0;
    fMaxIndex = fSize - 1;
    frequencyScale.resize(fSize);
    for (int i = 0; i < fSize; ++i) {
         frequencyScale[i] = fftEngine->getFrequency(i);
    }
    updateFMinMaxIndex();
}

void CSpectrogramPlot::updateFMinMaxIndex()
{
    float fMin = (float)displayParam.minFreq;
    float fMax = (float)displayParam.maxFreq;

    int fSize = (int)frequencyScale.size();
    fMinIndex = 0;
    fMaxIndex = fSize - 1;
    for (int i = 0; i < fSize; ++i) {
        if (frequencyScale[i] <= fMin) fMinIndex = i;
    }
    for (int i = fSize - 1; i >= 0; --i) {
        if (frequencyScale[i] >= fMax) fMaxIndex = i;
    }
}

void CSpectrogramPlot::setNewTimeScale(double tScale_)
{
    tScale = tScale_;
    tSize = qCeil(tScale * displayParam.sampleRate / (double)(fftSize / 2));
    tStep = (double)(fftSize / 2) / displayParam.sampleRate;
    timeScale.resize(tSize);
    for (int i = 0; i < tSize; ++i) {
        timeScale[i] = i * tStep;
    }
}

void CSpectrogramPlot::resetSpectrogram()
{
    for (int i = 0; i < (int)psdSpectrogram.size(); ++i) {
        psdSpectrogram[i].clear();
    }
    psdSpectrogram.resize(tSize);
    int fSize = (int)frequencyScale.size();
    for (int i = 0; i < tSize; ++i) {
        psdSpectrogram[i].resize(fSize);
        fill(psdSpectrogram[i].begin(), psdSpectrogram[i].end(), -10.0F);
    }
    psdSpectrum.clear();
    psdSpectrum.resize(fSize);
    fill(psdSpectrum.begin(), psdSpectrum.end(), -10.0F);

    psdRawImage = QImage(QSize(tSize, fSize), QImage::Format_ARGB32_Premultiplied);
    psdRawImage.fill(background);

    tIndex = 0;
    numValidTStepsInSpectrogram = 0;
    spectrogramFull = false;

    amplifierWaveformQueue.clear();
    amplifierWaveformRecordQueue.clear();
    waveformTimeStampQueue.clear();
}

void CSpectrogramPlot::resetQueue()
{
    amplifierWaveformQueue.clear();
    amplifierWaveformRecordQueue.clear();
    waveformTimeStampQueue.clear();
}

void CSpectrogramPlot::resizeEvent(QResizeEvent* /* event */) {
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    update();
}

void CSpectrogramPlot::mouseMoveEvent(QMouseEvent* event)  // Note: Must setMouseTracking(true) for this to work.
{
    bool mouseIsInFrame = scopeFrame.contains(event->pos());
    if (mouseIsInFrame || lastMouseWasInFrame) update();
    lastMouseWasInFrame = mouseIsInFrame;
}

void CSpectrogramPlot::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(&image);
    QRect imageFrame(rect());
    painter.fillRect(imageFrame, QBrush(Qt::black));

    CPlotDecorator plotDecorator(painter);

    if (displayParam.mode == Spectrogram) {
        // Plot spectrogram (frequency vs. time with color-coded amplitude).
        drawSpectrogram(painter, imageFrame, plotDecorator);
    }
    else {
        // Plot spectrum (amplitude vs. time).
        drawSpectrum(painter, imageFrame, plotDecorator);
    }

    QStylePainter stylePainter(this);
    stylePainter.drawImage(0, 0, image);
}


void CSpectrogramPlot::drawSpectrogram( QPainter& painter,  QRect& imageFrame, CPlotDecorator& plotDecorator)
{
    // Plot spectrogram (frequency vs. time with color-coded amplitude).
    int digitalFrameHeight = 0;
    int digitalFrameTopMargin = 0;
    int leftMargin = fontMetrics().horizontalAdvance("10000 Hz") + TickMarkLength + 10;
    int rightMargin = fontMetrics().horizontalAdvance("1000" + psdUnitsMicro) + TickMarkLength + 10;
    int bottomMargin = fontMetrics().height() + TickMarkLength + 10;
    int topMargin = fontMetrics().height() + 10;
    int colorBarWidth = 12;
    int colorBarSpacing = 12;


    scopeFrame = QRect(imageFrame.adjusted(leftMargin, topMargin, - (rightMargin + colorBarSpacing + colorBarWidth), -bottomMargin));
    QRect digitalFrame(scopeFrame.left(), digitalFrameTopMargin, scopeFrame.width(), digitalFrameHeight);
    CCoordinateTranslator ct(scopeFrame, 0.0, tScale, frequencyScale[fMinIndex], frequencyScale[fMaxIndex]);
    CCoordinateTranslator ctDigital(digitalFrame, 0.0, tScale, -0.5, 1.5);

    painter.setPen(Qt::white);
    QString title = customWaveName;
    if (!comments.isEmpty()) {
        title += "(" + comments + ")";
    }
    plotDecorator.writeLabel(title, (ct.xLeft() + ct.xRight()) / 2 , ct.yTop() - 1, Qt::AlignHCenter | Qt::AlignBottom);

    QRect CColorScaleFrame = QRect(scopeFrame.right() + colorBarSpacing, scopeFrame.top(), colorBarWidth, scopeFrame.height());
    colorScale->drawColorScale(painter, CColorScaleFrame);
    CCoordinateTranslator ctCColorScale(CColorScaleFrame, 0.0, 1.0, psdScaleMin, psdScaleMax);
    painter.setPen(Qt::white);
    plotDecorator.drawLabeledTickMarkRight("0.1" + psdUnitsMicro, ctCColorScale, -1.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkRight("1" + psdUnitsMicro, ctCColorScale, 0.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkRight("10" + psdUnitsMicro, ctCColorScale, 1.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkRight("100" + psdUnitsMicro, ctCColorScale, 2.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkRight("1000" + psdUnitsMicro, ctCColorScale, 3.0, TickMarkLength);

    painter.setPen(Qt::lightGray);
    for (double i = -2.0; i < 3.0; ++i) {
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_2, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_3, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_4, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_5, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_6, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_7, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_8, TickMarkMinorLength);
        plotDecorator.drawTickMarkRight(ctCColorScale, i + Log10_9, TickMarkMinorLength);
    }

    plotDecorator.drawTickMarkRight(ctCColorScale, 3.0 + Log10_2, TickMarkMinorLength);
    plotDecorator.drawTickMarkRight(ctCColorScale, 3.0 + Log10_3, TickMarkMinorLength);

    // Scale and insert power spectral density (PSD) image.
    painter.drawImage(scopeFrame, psdRawImage,
        QRect(0, psdRawImage.height() - fMaxIndex - 1, psdRawImage.width(), fMaxIndex - fMinIndex + 1));

    // Label frequency axis.
    painter.setPen(Qt::white);
    plotDecorator.drawLabeledTickMarkLeft(QString::number(displayParam.minFreq) + " Hz", ct, displayParam.minFreq, TickMarkLength);
    plotDecorator.drawLabeledTickMarkLeft(QString::number(displayParam.maxFreq) + " Hz", ct, displayParam.maxFreq, TickMarkLength);

    // Label time axis.
    for (int t = 0; t < tScale; ++t) {
        plotDecorator.drawLabeledTickMarkBottom(t, ct, t, TickMarkLength);
    }
    plotDecorator.drawLabeledTickMarkBottom(QString::number(tScale) + " s", ct, tScale, TickMarkLength);

    if (displayParam.showFreqMarker) {
        painter.setPen(Qt::red);
        double fMarker = displayParam.freqMarkerValue;
        plotDecorator.drawLabeledTickMarkLeft(QString::number(qRound(fMarker)) + " Hz", ct, fMarker, TickMarkLength);
        painter.setPen(Qt::black);
        plotDecorator.drawHorizontalAxisLine(ct, fMarker);
        for (int h = 0; h < displayParam.harmonicsNum; ++h) {
            double fHarmonic = (h + 2) * fMarker;
            if (fHarmonic >= displayParam.minFreq && fHarmonic <= displayParam.maxFreq) {
                painter.setPen(Qt::darkRed);
                plotDecorator.drawTickMarkLeft(ct, fHarmonic, TickMarkLength);
                painter.setPen(Qt::black);
                plotDecorator.drawHorizontalAxisLine(ct, fHarmonic);
            }
        }
    }
    
    QPoint cursor = mapFromGlobal(QCursor::pos());  // Get current cursor position.
    if (scopeFrame.contains(cursor)) {
        double fCursor = ct.realYFromScreenY(cursor.y());
        painter.setPen(Qt::yellow);
        plotDecorator.drawLabeledTickMarkLeft(QString::number(qRound(fCursor)) + " Hz", ct, fCursor, TickMarkLength);
        painter.setPen(Qt::black);
        plotDecorator.drawHorizontalAxisLine(ct, fCursor);
    }
}

void CSpectrogramPlot::drawSpectrum(QPainter& painter, QRect& imageFrame, CPlotDecorator& plotDecorator)
{
    const int LeftMargin = fontMetrics().horizontalAdvance("1000" + psdUnitsMicro) + TickMarkLength + 10;
    const int RightMargin = 25;
    const int BottomMargin = fontMetrics().height() + TickMarkLength + 10;
    const int TopMargin = fontMetrics().height() + 10;

    scopeFrame = QRect(imageFrame.adjusted(LeftMargin, TopMargin, -RightMargin, -BottomMargin));
    CCoordinateTranslator ct(scopeFrame, 0.0, tScale, frequencyScale[fMinIndex], frequencyScale[fMaxIndex]);

    ct.set(scopeFrame, frequencyScale[fMinIndex], frequencyScale[fMaxIndex], psdScaleMin, psdScaleMax);
    painter.setPen(Qt::white);
    painter.drawRect(ct.borderRect());    // Draw borders of scope.
    plotDecorator.writeLabel(customWaveName, ct.xLeft() + 2, ct.yTop() - 1, Qt::AlignLeft | Qt::AlignBottom);

    plotDecorator.drawLabeledTickMarkBottom(QString::number(displayParam.minFreq) + " Hz", ct, displayParam.minFreq, TickMarkLength);
    plotDecorator.drawLabeledTickMarkBottom(QString::number(displayParam.maxFreq) + " Hz", ct, displayParam.maxFreq, TickMarkLength);

    plotDecorator.drawLabeledTickMarkLeft("0.1" + psdUnitsMicro, ct, -1.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkLeft("1" + psdUnitsMicro, ct, 0.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkLeft("10" + psdUnitsMicro, ct, 1.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkLeft("100" + psdUnitsMicro, ct, 2.0, TickMarkLength);
    plotDecorator.drawLabeledTickMarkLeft("1000" + psdUnitsMicro, ct, 3.0, TickMarkLength);

    painter.setPen(Qt::lightGray);
    for (double i = -2.0; i < 3.0; ++i) {
        plotDecorator.drawTickMarkLeft(ct, i + Log10_2, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_3, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_4, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_5, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_6, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_7, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_8, TickMarkMinorLength);
        plotDecorator.drawTickMarkLeft(ct, i + Log10_9, TickMarkMinorLength);
    }
    plotDecorator.drawTickMarkLeft(ct, 3.0 + Log10_2, TickMarkMinorLength);
    plotDecorator.drawTickMarkLeft(ct, 3.0 + Log10_3, TickMarkMinorLength);

    painter.setPen(QColor(72, 72, 72));
    for (int y = -1; y < 4; ++y) {
        plotDecorator.drawHorizontalAxisLine(ct, y, 1);
    }

    if (displayParam.showFreqMarker) {
        painter.setPen(Qt::red);
        double fMarker = displayParam.freqMarkerValue;
        plotDecorator.drawVerticalAxisLine(ct, fMarker);
        plotDecorator.drawLabeledTickMarkBottom(QString::number(qRound(fMarker)) + " Hz", ct, fMarker, TickMarkLength);
        painter.setPen(Qt::darkRed);
        for (int h = 0; h < displayParam.harmonicsNum; ++h) {
            double fHarmonic = (h + 2) * fMarker;
            if (fHarmonic >= displayParam.minFreq && fHarmonic <= displayParam.maxFreq) {
                plotDecorator.drawVerticalAxisLine(ct, fHarmonic);
                plotDecorator.drawTickMarkBottom(ct, fHarmonic, TickMarkLength);
            }
        }
    }

    QPoint cursor = mapFromGlobal(QCursor::pos());  // Get current cursor position.
    if (scopeFrame.contains(cursor)) {
        painter.setPen(Qt::yellow);
        double fCursor = ct.realXFromScreenX(cursor.x());
        plotDecorator.drawVerticalAxisLine(ct, fCursor);
        plotDecorator.drawLabeledTickMarkBottom(QString::number(qRound(fCursor)) + " Hz", ct, fCursor, TickMarkLength);
    }

    painter.setClipRect(ct.clippingRect());

    int polylineLength = fMaxIndex - fMinIndex + 1;
    QPointF* polyline = new QPointF[polylineLength];
    int i = 0;
    for (int fIndex = fMinIndex; fIndex <= fMaxIndex; ++fIndex) {
        polyline[i++] = QPointF(ct.screenXFromRealX(frequencyScale[fIndex]), ct.screenYFromRealY(psdSpectrum[fIndex]));
    }
    painter.setPen(Qt::green);
    painter.drawPolyline(polyline, polylineLength);
    delete[] polyline;
    painter.setClipping(false);
}
END_NX_NAMESPACE
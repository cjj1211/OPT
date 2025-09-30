/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_SPECTROGRAM_PLOT_H
#define __C_SPECTROGRAM_PLOT_H
#include "nxspectrogram_global.h"
#include <string>
#include <deque>
#include <qevent.h>
#include <QtWidgets/QWidget>
#include "CColorScale.h"

#define SPECPLOT_X_SIZE 600
#define SPECPLOT_Y_SIZE 550

BEGIN_NX_NAMESPACE
enum SpectrogramMode {
    Spectrogram = 0,
    Spectrum
};

struct SpectrogramParam {
    bool showFreqMarker{true};
    double freqMarkerValue{50};
    double fftSize{256}; //    128 256 512 1024 2048 4096 8192
    double tScale{10}; // 2s 5s 10s
    double minFreq{0};
    double maxFreq{200};
    double sampleRate{1000} ;
    int harmonicsNum{0};
    SpectrogramMode mode{ Spectrogram };
};

class CFastFourierTransform;
class CPlotDecorator;

class CSpectrogramPlot : public QWidget
{
    Q_OBJECT
public:
    explicit CSpectrogramPlot(QWidget* parent = nullptr);
    ~CSpectrogramPlot();

    QString getCustomWaveName();
    void setWaveform(const QString& customWaveName_, const QString& _comments, double _sampleRate);
    bool updateWaveforms( int numSamples, const QVector<float>& data, const QVector<uint64_t>& timestamps);
    void resetSpectrogram();
    void resetQueue();

    double getDeltaTimeMsec() const { return 1000.0 * tStep; }
    double getDeltaFreqHz() const { return frequencyScale[1]; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setDisplayMode(const SpectrogramMode _mode);
    void setFftSize(const double _fftSize);    
    void setFreqMin(const double _freqMin);    
    void setFreqMax(const double _freqMax);    
    void setFreqMarkerValue(const double _markerValue);    
    void setShowMarker(const bool _enable);    
    void setNumHarmonics(const int num);    
    void setTScale(const int scale);
    void setBackground(const QColor color);

public slots:
    void updateFromState();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void drawSpectrogram( QPainter& painter,  QRect& imageFrame, CPlotDecorator& plotDecorator);
    void drawSpectrum(QPainter& painter, QRect& imageFrame, CPlotDecorator& plotDecorator);
    void setNewFftSize(int fftSize_);
    void updateFMinMaxIndex();
    void setNewTimeScale(double tScale_);

private:
    QString customWaveName;
    QString comments;
    std::deque<float> amplifierWaveformQueue;
    std::deque<float> amplifierWaveformRecordQueue;
    std::deque<uint64_t> waveformTimeStampQueue;
    QVector<float> dataBuffer;

    SpectrogramParam displayParam;

    CFastFourierTransform* fftEngine;
    int fftSize;
    int sampleCounter{0};
    float spikeAvgHight;

    double tScale;
    int tSize;
    int tIndex;
    int numValidTStepsInSpectrogram;
    bool spectrogramFull;
    double tStep;

    float* fftInputBuffer;
    std::vector<float> frequencyScale;
    int fMinIndex;
    int fMaxIndex;
    std::vector<float> timeScale;
    std::vector<float> psdSpectrum;
    std::vector<std::vector<float>> psdSpectrogram;
    QImage psdRawImage;

    double psdScaleMin;
    double psdScaleMax;
    CColorScale* colorScale;
    QImage image;
    QRect scopeFrame;
    bool lastMouseWasInFrame;
    QString psdUnitsMicro;
    QColor background;
};

END_NX_NAMESPACE
#endif
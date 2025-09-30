/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_SPECTROGRAM_H
#define __C_SPECTROGRAM_H
#include "nxspectrogram_global.h"

#include <QWidget>
#include <memory>
#include <mutex>

namespace Ui {
    class CSpectrogram;
}

BEGIN_NX_NAMESPACE
class CSpectrogramPlot;
class NX_SPECTROGRAM_EXPORT CSpectrogram: public QWidget
{
    Q_OBJECT
public:
    CSpectrogram(QWidget* parent = nullptr);
    ~CSpectrogram() override;
    
    void activate();
    void changeCurrentChannel(const QString& customChannelName, const QString& comments, const double sampleRate);
    void resetBuffer();
    void setBackground(const QColor color);
    QString getCurentCustomWaveName();
    void updateWaveforms(int numSamples, const QVector<float>& data, const QVector<uint64_t>& timestamps);

public slots:
    void setNumFftPoints(int index);
    void changeDisplayMode(int index);
    void setFMin(int fMin);
    void setFMax(int fMax);
    void setFMarker(int fMarker);
    void toggleShowFMarker(bool enabled);
    void setNumHarmonics(int num);
    void setTimeScale(int index);

private:
    void updateDeltaTimeFreqLabels();

private:
    std::unique_ptr<Ui::CSpectrogram>   ui;
    const int                           FSpanMin = 10;
    std::mutex                          plotMutex;
    CSpectrogramPlot*                   specPlot;
};

END_NX_NAMESPACE
#endif

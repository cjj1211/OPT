/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include <QObject>
#include <QWidget>
#include <QQueue>

#include "CCoordinateTranslator.h"

class CSpikePlot  : public QWidget
{
    Q_OBJECT

public:
    CSpikePlot(const QString& channelName_, const int sampleRate_, QWidget *parent = nullptr);
    ~CSpikePlot() override;

signals:
    void signalThresholdChanged(int value);

public slots:
    void slotUpdateTimeScale(const int timeScale_); // ms 毫秒
    void slotSetWaveformName(const QString& channelName, const int sampleRate_);
    void slotSetMaxSpikeCount(const int count);
    void slotTakeSnapshot();
    void slotUpdateWaveforms(QVector<float> waveformSegment);
    void slotClearSpikes();
    void slotClearSnapshot();
    void slotUpdateThreshold(int threshold_);

protected:
    void paintEvent(QPaintEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateCoordinateTranslator();

private:
    int samplesPreDetect;
    int samplesPostDetect;
    int sampleRate;
    double yScale{ 500 }; // 50 100 200 500 1000 2000 5000 uV
    double timeScale{2}; // 2\4\6 ms 毫秒
    double tStepMsec; // ms 毫秒

    QString wavename{ "A-111"};
    int showSpikeCounts{ 20 };
    int threshold{ 100 };

    QQueue<QVector<float>> spikeSegments;
    QQueue<QVector<float>> snapshotSpikeSegments;
    QVector<float> preSamples;
    QQueue<float> rmsBuffer;

    QRect scopeFrame;
    QImage image;

    double latestRmsCalculation { 0 };
    int latestSpikeRateCalculation { 0 };

    CCoordinateTranslator ct;
};

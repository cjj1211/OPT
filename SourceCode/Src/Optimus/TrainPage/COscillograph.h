/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_OSCILLOGRAPH_H
#define __C_OSCILLOGRAPH_H
#include "../Global.h"
#include "ui_Oscillograph.h"
#include <NxWaveformWidget/CommunicationDto.h>
#include <QWidget>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <string>
#include <memory>
#include "NxAppService/COscillographService.h"

class CSpikeScope;
BEGIN_NX_NAMESPACE
class MultiColumnDisplay;
class COscillographService;
class CSpectrogram;
class CProgressDialog;
enum OscillographMode {
    OS_SIGNAL = 0,
    OS_SPECTROGRAM
};

class COscillograph : public QWidget
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER

private:
    enum OscillographComponentID {
        ID_SIGNAL = 0,
        ID_SNR,
        ID_RMS,
        ID_SPECTRUM
    };


public:
    COscillograph(COscillographService* _service, const bool _isPlayback, QWidget* parent = nullptr);
    ~COscillograph() override;
    bool init();
    void resetWaveform() const;

private:
    void initWaveformChannels();
    void initChannelImpedance() const;
    bool initBoard() const;
    void initPlayback() const;
    void updateTimeAxisInSweepMode(const uint64_t startTimestamp, const uint64_t endTimestamp);
    void updateRunTime(const uint64_t endTimestamp, const double sampleRate);

    static QColor channelColor(int colorIndex, int numColors);

    void showProgressDialog(const QString& content);
    void updateProgressContent(const QString& content) const;
    void closeProgressDialog();
    void changeTimeScale() const;

signals:
    void signalUpdateDisabledWaveforms(const QStringList&);
    void signalUpdateTimestamp(const uint64_t);
    void signalServerIsStoped();
    void signalExitOscillograph();

public slots:
    void slotUpdateWaveform(bool, bool, int, uint64_t, uint64_t, bool, QVector<QMap<std::string, std::string>>);
    void slotUpdateSelectedAmpData(const QString& nativeName, int numSamples, const  QVector<float>& data, const  QVector<uint64_t>& timestamps);
    void slotUpdateDisabledChannels();
    void slotChannelPlanToggled(const QString& planUid, bool enabled);
    void slotEnableChannel(const QString& customWavename);
    void slotSpectrogramResolutionChanged(int index);
    void slotSpectrogramTimeScaleChanged(int index) const;
    void slotSpectrogramModeChanged(int index) const;
    void slotSpectrogramMinFreqChanged(int val) const;
    void slotSpectrogramMaxFreqChanged(int val) const;
    void slotToggleSpikeScope();
    void slotSpikeScopeClosed();
    void slotResetSampleRate();

private slots:
    void slotSignalBtnClicked() const;
    void slotSNRBtnClicked();
    void slotRMSBtnClicked();
    void slotSpectrumBtnClicked();
    void slotPaperSpeedChanged();
    void slotYScaleChanged();
    void slotBoardInited(bool, QMap<QString, QString>);
    void slotUpdateDisplayParams();
    void slotChangeSelctedWaveName(QString nativeName);

private:
    std::unique_ptr<Ui::Oscillograph> ui;
    MultiColumnDisplay* waveforms;
    CSpectrogram* spectrogram;
    CSpikeScope* spikeScope;
    COscillographService* service;
    CProgressDialog* progressDialog;
    OscillographComponentID currentCheckedBtn;
    QRect waveformRegion;
    int sampleRate;
    bool isInited;
    bool isPlayback;

    UiDisplayDto dto;
    QMap<QString, QVector<float>*> samplesMap;
};
END_NX_NAMESPACE
#endif
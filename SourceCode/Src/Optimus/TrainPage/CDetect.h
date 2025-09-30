/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DETECT_H
#define __C_DETECT_H
#include "../Global.h"
#include "../CChannelSelectorCard.h"
#include "../CHidedChannelCard.h"
#include "ui_Detect.h"
#include <QWidget>
#include <memory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace NX
{
    class CExportEEGDialog;
}

class CEventWidget;
BEGIN_NX_NAMESPACE
class COscillograph;
class COscillographService;
class CElectrodePreview;
class CImpedanceWidget;
class CHeatMap;
class CDetectToolbar;
class CPlaybackToolbar;
class CProgressDialog;
class CDetect: public QWidget
{
	Q_OBJECT
		QT_TRANSLATE_HANDLER
public:
    CDetect(const QString& currentPatientUid, QWidget* parent = nullptr);
    CDetect(const QString& currentPatientUid, const QString& recordUid,QWidget* parent = nullptr);
    ~CDetect() override;

    void initOscillograph();
    bool isRecording();
    void stopRecord();
private:
    void initChannelSelectorCard();
    void initHidedChannelCard();
    void updateEnabledStatus();

signals:
    void signalPlaybackExit();
    void signalExitOscillograph();

public slots:
    void slotRun(bool);
    void slotServerStoped();
    void slotRecord(bool);
    void slotImpedance();
    void slotCloseImpedance();
    void slotCloseHeatMap();
    void slotChannelMap();
    void slotUpdateDisabledWaveforms(const QStringList& customWavenames);
    void slotButtonClick(const QString& name, int count);

    void slotJumpTo(const uint64_t timeInSecs) const;
    void slotPlaybackImpedance();
    void slotPlaybackExport();

    void slotDeleteEvent();

    void slotCompressProgress(QString fileName, int currentIndex, int allIndex);
private:
    void loadEventRecorder(const QString &path );
    void saveEventRecorder();
    void initTableWidget();
    QString setEventTime(const QString& time);
    void checkEventRecord();
    void slotChannelPlanToggled(const QString&, bool enabled);
    void doubleClickEventTable() const;
private:
    std::unique_ptr<Ui::Detect> ui;
    QString patientUid;
    CDetectToolbar* detectToolbar;
    CPlaybackToolbar* playbackToolbar;
    COscillograph* oscillograph;
    CExportEEGDialog* exportDialog;
    CImpedanceWidget* impedance;
    COscillographService* service;
    CHidedChannelCard* hideChannelCard;
    CHeatMap* heatMap;
    QVector<CChannelSelectorCard*> channelSelectorCardVec;
    CEventWidget* eventWidget;
    CProgressDialog* progressDialog;
    QJsonArray jsonDataArray;
    QString configFilePath;
    QStringList eventStartTime;
    QString     startTime;
    int         trainTimeLen;
    int         currentModel=0;
};
END_NX_NAMESPACE
#endif
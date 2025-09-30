/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_IMPEDANCE_H
#define __C_IMPEDANCE_H
#include "Global.h"
#include "ui_CImpedanceWidget.h"
#include <QWidget>
#include <QVector>
#include <QMap>
#include <QFuture>
#include <memory>

BEGIN_NX_NAMESPACE
class COscillographService;
class CChannelSelectorCard;
class CColorGradientWidget;
class CElectrodePreview;
class CProgressDialog;
class CImpedanceWidget  : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CImpedanceWidget(COscillographService* sc, bool canMeasure = true, QWidget *parent = nullptr);
    ~CImpedanceWidget() override;

    void updateEnabledStatus();
private:
    void initChannelSelectorCard();
    void updateImpedanceList();

    static QString filename2itemName(QString filename);

signals:
    void signalMeasureImpedanceDone();
    void signalClosed();
    void signalUpdateMeasureStatus(bool isCompleted, int progress);
    void signalChannelPlanToggled(const QString&, bool);

private slots:
    void slotMeasureImpedance();
    void slotUpdateChannelMap();
    void slotChannelPlanToggled(QString&, bool);
    void slotCurrentItemChanged(QListWidgetItem*, QListWidgetItem*);
    void slotUpdateMeasureStatus(bool isCompleted, int progress);

private:
    std::unique_ptr<Ui::CImpedanceWidget> ui;
    COscillographService* service;
    QVector<CChannelSelectorCard*> channelSelectorCardVec;
    CColorGradientWidget* colorGradient;
    CElectrodePreview* channelMap;
    CProgressDialog* progressDialog;
    QMap<QString, bool> enabledPort;
    QMap<QString, QString> impedanceHisMap;
    QString detectorInfos;
    bool isMeasuring;
    QFuture<void> updateMeasureStatusThread;
};

END_NX_NAMESPACE

#endif
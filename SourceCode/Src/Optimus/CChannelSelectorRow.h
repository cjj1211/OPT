/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_CHANNEL_SELECTOR_ROW_H
#define __C_CHANNEL_SELECTOR_ROW_H
#include "Global.h"
#include "ui_CChannelSelectorRow.h"
#include "NxEntity/ChannelPlan.h"
#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
class COscillographService;
class CFilterSettingDialog;
struct ChannelPlan;
class CChannelSelectorRow: public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CChannelSelectorRow(COscillographService* oscillographService,ChannelPlan* plans, const bool showSignalTypeAndSetting, QWidget* parent = nullptr);
    ~CChannelSelectorRow() override;

    bool isChecked();
    void updateEnabledStatus();

signals:
    void signalChannelPlanToggled(const QString& uid, bool enabled);
    void signalSampleRateChanged();

private slots:
    void slotCheckedFromUi(bool enabled);

public slots:
    void slotChannelPlanToggled(bool enabled);

private slots:
    void slotSettingBtnClicked();
    void slotSignalTypeChanged(int);

private:
    std::unique_ptr<Ui::CChannelSelectorRow> ui;
    COscillographService* service;
    ChannelPlan* channelPlan;
    CFilterSettingDialog* filterSettingDialog;
};

END_NX_NAMESPACE
#endif
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_FILTER_SETTING_DIALOG_H
#define __C_FILTER_SETTING_DIALOG_H
#include "Global.h"
#include "ui_CFilterSettingDialog.h"
#include "CDragableDialog.h"
#include <QDialog>
#include <memory>

BEGIN_NX_NAMESPACE
struct ChannelPlan;
class COscillographService;
class CFilterSettingDialog: public CDragableDialog
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CFilterSettingDialog(COscillographService* oscillographService, ChannelPlan* plan,QWidget* parent = nullptr);
    ~CFilterSettingDialog() override;

signals:
    void signalSampleRateChanged();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void slotSampleRateChanged(int index);
    void slotFilterChanged();
    void slotConfirm();
private:
    std::unique_ptr<Ui::CFilterSettingDialog> ui;
    COscillographService* service;
    ChannelPlan* channelPlan;
    bool isSampleRateChanged;
    bool isFilterChanged;
};

END_NX_NAMESPACE
#endif
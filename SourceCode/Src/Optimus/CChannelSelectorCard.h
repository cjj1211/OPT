/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_CHANNEL_SELECTOR_CARD_H
#define __C_CHANNEL_SELECTOR_CARD_H
#include "Global.h"
#include "CChannelSelectorRow.h"
#include "ui_CChannelSelectorCard.h"
#include "NxEntity/ChannelPlan.h"
#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
class COscillographService;
struct ChannelPlan;
class CChannelSelectorCard: public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CChannelSelectorCard(COscillographService* oscillographService, QVector<ChannelPlan*> plans, const bool showSignalTypeAndSetting, QWidget* parent = nullptr);
    ~CChannelSelectorCard() override;

    void updateEnabledStatus();
    static QString getAreaName(const ChannelPlan& plan);
public slots:
    void slotAreaCheckBoxChanged(bool enabled);
    void slotChannelPlanToggled(const QString&, bool enabled);

signals:
    void signalChannelPlanToggled(const QString& planUid, bool enabled);
    void signalSampleRateChanged();

private:
    std::unique_ptr<Ui::CChannelSelectorCard> ui;
    COscillographService* service;
    QMap<std::string,CChannelSelectorRow*> rowsMap;
    QVector<ChannelPlan*> channelPlans;
};

END_NX_NAMESPACE
#endif
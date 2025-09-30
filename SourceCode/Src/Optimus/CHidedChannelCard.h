/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_HIDED_CHANNEL_CARD_H
#define __C_HIDED_CHANNEL_CARD_H
#include "Global.h"
#include "ui_CHidedChannelCard.h"
#include <QWidget>
#include <QCheckBox>
#include <QSet>
#include <memory>

class QPropertyAnimation;
BEGIN_NX_NAMESPACE
class COscillographService;
class CHidedChannelCard: public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CHidedChannelCard(COscillographService* oscillographService, QWidget* parent = nullptr);
    ~CHidedChannelCard() override;

    void hideChannel(const QString& customWavename);
    void showChannel(const QString& customWavename);
    QSet<QString> getHidedWavenaems();
    void saveHidedChannels();
private:
    void addHidedChannelWidget(const QString& customWavename);
    void removeHidedChannelWidget(const QString& customWavename);

public slots:
    void slotHidedWaveChecked();

signals:
    void signalShowChannel(const QString& customWavename);

private:
    std::unique_ptr<Ui::CHidedChannelCard> ui;
    QMap<QString, QCheckBox*> hidedChannelMap;// key: customName
    QMap<QString, QPropertyAnimation*> hidedChannelAnimations;
    QMap<QString, QPropertyAnimation*> showChannelAnimations;
    COscillographService* service;
    QSet<QString> hidedCustomNames;
};

END_NX_NAMESPACE

#endif

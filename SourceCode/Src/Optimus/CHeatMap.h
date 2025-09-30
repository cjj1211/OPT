/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_HEAT_MAP_H
#define __C_HEAT_MAP_H
#include "Global.h"
#include <QWidget>
#include <QMap>
#include <QFuture>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class CHeatMap; };
QT_END_NAMESPACE

BEGIN_NX_NAMESPACE

class COscillographService;
class CColorGradientWidget;
class CElectrodePreview;

class CHeatMap : public QWidget
{
    Q_OBJECT
public:
    CHeatMap(COscillographService* sc, QWidget *parent = nullptr);
    ~CHeatMap();

private:
    void initChannelMap();
    void setHeatScaleChannels();
    void startMeasureHeat();

signals:
    void signalClosed();
    void signalHeatValueLoaded();

private slots:
    void slotUpdateChannelMap();
    void searchChannelMap();
    void showCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber, QString currentTabIndex);

private:
    std::unique_ptr<Ui::CHeatMap> ui;

    COscillographService* service;
    CColorGradientWidget* colorGradient;
    CElectrodePreview* channelMap;
    QMap<QString, QString> groupNameMap;
    QString currentGroup;

    QFuture<void> loadHeatScaleThread;
    bool isLoadingHeatScales;
};

END_NX_NAMESPACE
#endif
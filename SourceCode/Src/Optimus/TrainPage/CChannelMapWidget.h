/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 :  崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __C_CHANNELMAPWIDGE_H
#define __C_CHANNELMAPWIDGEN_H
#include "ui_CChannelMapWidget.h"
#include "../Global.h"

#include <QWidget>

BEGIN_NX_NAMESPACE
class CChannelNameService;
struct ChannelName;
struct ChannelPlan;
class CChannelPlanService;
class CElectrodePreview;

class CChannelMapWidget : public QWidget
{
    Q_OBJECT

public:
    CChannelMapWidget(QWidget *parent = nullptr);
    ~CChannelMapWidget();
    void  initChannelMap(const QString& ChannelPlanFK);
public :
    QList<QPair <QString, QString>>  channelMap;
    QString patientFK;
    int currentTab;
signals:
    void hideChannelMapWidget();
private:
    void channelMapWidgetHide();
    void searchChannelMap();
    void saveChannelMap();
    void showCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber, QString currentTabIndex);
    void updateCurrentChannelName(int currentButtonX, int currentButtonY, QString currentTabIndex, int channelnumber);
private:
    std::unique_ptr <Ui::CChannelMapWidget> ui;
    CChannelNameService* channelNameService;
    CChannelPlanService* channelPlanService;
    int currentCoordX=2;
    int currentCoordY = 2;
    CElectrodePreview* electrodePreview;
    QString channelPlanFK;
 
    QPair<QPair<int, int>, QPair<QString, QString>>currentSelectChannelName;
};

END_NX_NAMESPACE
#endif // CCHANNELMAPWIDGET_H

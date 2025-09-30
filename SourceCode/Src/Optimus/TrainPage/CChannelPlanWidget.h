/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 :  崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __C_CHANNEL_PLAN_H
#define __C_CHANNEL_PLAN_H

#include "../Global.h"
#include "ui_CChannelPlanWidget.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "NxElectrodeMap/CElectrodeMapDataStructure.h"
#include <QWidget>
#include <QPair>
#include <QComboBox>
#include <QDoubleValidator>
#include <QListView>
#include <QVector3D>
#include <memory>

BEGIN_NX_NAMESPACE
class CChannelPlanService;
class CChannelNameService;
struct ChannelPlan;
struct ChannelName;
class CElectrodeSettingInterface;
class CBrainAreaWidget;
class CProgressDialog;

enum Toponym {
    FrontalLobe,
    TemporalLobe,
    ParietalLobe,
    OccipitalLobe,
    LobusInsularis,
    LimbicSystem,
    AC,
    FrontoparietalIslJunction

};

class CChannelPlanWidget : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
signals:
    void channelMapShow(int currentrow);

public:
    CChannelPlanWidget(QString  CurrentPatientUid,QWidget* parent = nullptr);
    ~CChannelPlanWidget() override;
    void saveChannelPlan();
public:
    QString channelPlanFK;
    QList<QPair <QString, QString>>  channelMap;
    QString currentPatientUid;
    int saveState = 0;

signals:
    void signalChannelPlanChanged();

public slots:
    void deleteChannelPlan();
    void slotBrainAreaInited();

private:
    void setTableHeader() const;

    void setBrainHemisphere();
    void setLobe(int hemisphereIndex);
    void setArea (int hemisphereIndex,int lobeIndex);
    
    void initTable();
    QString getNextUppercaseLetter(QString currentLetter);
    bool isUpdate(const ChannelPlan& currentChannel, const ChannelPlan& initChannel);
    void setTableWidgetStyle();
    void showChannelMap();
    void findXML(QString path);
    void updateChannelName(ChannelPlan currentChannel, ChannelPlan initChannel);
    void updateChannelName(ChannelPlan currentChannel);
    QString getHemisphereTrName(const ChannelPlan& plan);
    QString getLobeTrName(const ChannelPlan& plan);
    QString getAreaTrName(const ChannelPlan& plan);

    std::string getPositionByUI(const int row) const;
    std::string getLobeByUI(const int row) const;
    std::string getAreaByUI(const int row) const;
    void setDetectorTransformToPlan(ChannelPlan& plan);

public slots:
    void changeCurrentFpgaPort(int index);
    void changeCurrentChanelCounts(int index);
    void insertRow();
    void slotHemisphereChanged(int index);
    void slotLobeChanged(int index);
    void slotAreaChanged(int index);
    void slotUpdateDetectorTransform(QString planIndex, QVector3D translate, QVector3D rotate);
    void slotUpdateProbeTransform(QString planIndex, QVector3D translate, QVector3D rotate);

private:
    std::unique_ptr<Ui::CChannelPlanWidget> ui;
    CChannelPlanService* channelService;
    CChannelNameService* channelNameService;

    QStringList signalType = { "Raw","LFP","Spike" };
    QStringList notch = {QString::fromLocal8Bit("无"),"50","60"};
    QStringList samplingRate = {"500","800","1000","1250","1500", "2000", "2500", "3000", "3333", "4000", "5000", "6250", "8000", "10000", "12500","15000","20000", "25000", "30000"};
    QString styleSheet = "QComboBox { background: transparent; border: none; font-size: 15px; color: #ffffff; font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400; } "
        "QComboBox::item:selected { background-color: rgba(54, 157, 251, 0.60); } "
        "QComboBox::down-arrow { image: url(:/Optimus/images/trainPage/down.svg); border: none; width: 8px; height: 6px; } "
        "QComboBox::drop-down { subcontrol-position: right; border: none; } "
        "QComboBox QAbstractItemView { background-color: #283c82; color: #ffffff; selection-background-color: rgba(54, 157, 251, 0.60); border: none; } "
        "QComboBox::item { background-color: transparent; color: #ffffff; font-size: 15px; font-weight: bold; border: none; height: 20px; }";
    QString lienStyleSheet = { "QLineEdit { "
        "background: transparent; "
        "border: none; "
        "font-size: 15px; "
        "color: #ffffff; "
        "font-family: 'PingFangSC-Regular', 'PingFang SC', sans-serif, 'PingFangSC-Regular', 'PingFang SC', sans-serif-400; "
    "} "
    "QLineEdit:focus { "
        "background-color: #1FE9F4; "
        "color: #162455; "
    "}" };
    QStringList brainHemisphere;
    QStringList detectorType;
    QStringList areaType;
    QStringList lobeType;
    QStringList fpgaPort;
    QStringList currentFpgaPort;
    QList<int> fpgaList;
    QString type = "A"; //存数据库里最后的字母
    std::vector<ChannelPlan> channelPlanVec;//界面初始化查出的数据用于进行保存时对比
    QString buttonStyle = { "QPushButton { background: #1890ff; border - radius: 4px; font - size: 16px; color: rgba(255, 255, 255, 0.85); } QPushButton:hover { background: rgba(24, 144, 255, 0.8); border - radius: 4px; font - size: 16px; color: rgba(255, 255, 255, 1.0); } QPushButton:disabled { background: #cccccc; color: rgba(255, 255, 255, 0.5); border: none; }" };
    ElectrodeMapSettings* setting ;
    QList<QPair<QString, int>> allMapInfo;
    CElectrodeSettingInterface *electrodeSettingInterface;
    CBrainAreaWidget* brainArea;

    QDoubleValidator* validator;
    QList< QVector<ElectrodeSite> >electrodeSites;
};
END_NX_NAMESPACE
#endif
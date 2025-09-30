/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __PATIENT_PAGE_H
#define __PATIENT_PAGE_H
#include "../Global.h"
#include "ui_TrainPage.h"
#include "NxEntity/StandardTreatment.h"
#include <QWidget>
#include <QVector>
#include <memory>
#include "NxEntity/Patient.h"
namespace NX
{
    class CPatientInfo;
}

BEGIN_NX_NAMESPACE
class CTreatmentRecordService;
class CChannelMapWidget;
class CTrainRecord;
class CDetect;
class CChannelPlanWidget;
class CTreatmentCfg;
class CBinding;
class CTrainService;
class CWebSocketService;
class  CStandardTreatmentService;

class TrainPage final : public QWidget {
    Q_OBJECT
        QT_TRANSLATE_HANDLER
public:
    TrainPage(QWidget* parent = nullptr);
    ~TrainPage() override;

    bool isRecording() const;
    void stopRecord() const;
public slots:
    void slotTogglePanel();
    void slotSetComponentStatus() const;

private slots:
    void slotChannelPlanBtnClicked();
    void slotRealtimeDetectBtnClicked();
    void slotTreatmentCfgBtnClicked();

    void slotBindingBtnClicked();
    void slotTrainRecordBtnClicked();
    void slotRealtimeDetectJumpToPrevious();
    void slotStartTraining();

    void slotEditPatient();
    void slotEditPatientExit(const QString&);


private:
    void checkStepBtn(TrainCommponentID id);
    void hideChannelMap() const;
    void showChannelMap(int currentRow);
    void updatePatientInfo();
    void showAvatar(const std::vector<char>& avatar, char gender) const;
    void showIllStage(TypeOfIllness illType, char illStage) const;
    void showGender(GenderEmu gender) const;

private:
    std::unique_ptr<Ui::TrainPage> ui;
    CTrainService* service;
    CChannelMapWidget* channelMapWidget;
    CTrainRecord* recordComponent;
    CChannelPlanWidget* channelPlanComponent;
    CDetect* detectComponent;
    CTreatmentCfg* treatmentCfgComponent;
    CBinding* bindingComponent;
    CPatientInfo* patientInfo;

    /* 保存ComponentID 与 在stackWidget中componentIndex映射*/
    QMap<TrainCommponentID, int> componentIdIndexMap;
    TrainCommponentID currentComponentID;
    TrainCommponentID preComponentID;
    QString currentPatientUid;
    bool isFoldPanel;
    CWebSocketService* webSocketService;
    CStandardTreatmentService* standardTreatmentService;
    StandardTreatment st;
    bool isInitOscillograph = false;
};
END_NX_NAMESPACE

#endif

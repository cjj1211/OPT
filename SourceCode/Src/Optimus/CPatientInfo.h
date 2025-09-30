/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_PATIENT_INFO_H
#define __C_PATIENT_INFO_H
#include "Global.h"
#include <QWidget>
#include <QResizeEvent>
#include "ui_CPatientInfo.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CPatientInfo; };
QT_END_NAMESPACE

class QButtonGroup;
BEGIN_NX_NAMESPACE

class CPatientInfoService;
class CRecordFace;
class CIllnessInfo;
struct Patient;
struct IdentityInfo;
class CPatientInfo : public QWidget
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER;

    enum BG_Gender {
        Male = 0,
        Female
    };

public:
    CPatientInfo(const QString& prePageName, bool canBeDelete, const QString& patientUid = "", QWidget* parent = nullptr);
    ~CPatientInfo();

protected:
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private:
    void showAvatar() const;

signals:
    void signalExitCreatePatient();
    void signalEditPatientExit(QString uid);
    void signalPatientDeleted();

private slots:
    void slotGenderSelected(int, bool);
    void slotALSSelected(int, bool);
    void slotSeizureRadioSlected(int, bool);
    void slotDepressionRadioSelected(int, bool);
    void slotSCIRadioSelected(int, bool);
    void slotPDRadioSelected(int, bool);
    void slotBlindnessRadioSelected(int, bool);
    void slotSickStateChanged(int);
    void slotNameChanged(QString) const;
    void slotCaseIDChanged(QString);
    void slotBirthdayChanged(QDate);
    void slotDelete();
    void slotSave();
    void slotBinding();
    void slotCapture();
    void slotUpdateAvator(QImage img);
    void slotDiseaseEditorChanged();
    void slotInsertDiseaseInfo();
    void slotTabChanged(int);

private:
    Ui::CPatientInfo* ui;
    CPatientInfoService* service;
    IdentityInfo* identityInfo;
    Patient* patientInfo;
    CRecordFace* recordFace;
    QString curPatientUid;
    BG_Gender gender;
    QDate birthday;
    QButtonGroup* genderGroup;
    QButtonGroup* alsGroup; // 渐冻症
    QButtonGroup* seizuresGroup; // 癫痫
    QButtonGroup* depressionGroup; // 抑郁症
    QButtonGroup* sciGroup; // 脊髓损伤
    QButtonGroup* pdGroup; // 帕金森
    QButtonGroup* blindnessGroup; // 失明
    QButtonGroup* stateGroup;
    QRegExpValidator* nameValidator;
    QRegExpValidator* caseIDValidator;
    QMap<QString, CIllnessInfo*> illnessInfoWidgetMap;

    int alsState, seizuresState, depressionState, sciState, pdState, blindnessState;
    int illType;
    bool isNameValided, isCaseIdValided;
    bool isEditPatient;
};

END_NX_NAMESPACE
#endif
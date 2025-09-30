/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TREATMENT_CFG_H
#define __C_TREATMENT_CFG_H
#include "../Global.h"
#include "ui_TreatmentCfg.h"
#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
class   CStandardTreatmentService;
struct StandardTreatment;
class CCourseAllocationWidget;
class CTreatmentCfg : public QWidget
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER
public:
    CTreatmentCfg(QWidget* parent = nullptr);
    ~CTreatmentCfg() override;
    void setPatientInfo(QString CurrentPatientUid, int CurrentDiseaseName, int CurrentStaging);
    void initWidget();
private:
    std::unique_ptr<Ui::TreatmentCfg> ui;
    CCourseAllocationWidget* courseAllocationWidget;;
    QString currentPatientUid="123";
    int currentDiseaseName=0;
    int currentStaging=0;
};

END_NX_NAMESPACE
#endif
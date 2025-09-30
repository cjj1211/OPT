/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __STANDARD_TREATMENT_PAGE_H
#define __STANDARD_TREATMENT_PAGE_H
#include "../Global.h"
#include "ui_StandardTreatmentPage.h"

#include <QWidget>
#include <memory>
#include <QMetaType>


BEGIN_NX_NAMESPACE
class   CStandardTreatmentService;
struct StandardTreatment;
class CCourseAllocationWidget;
struct StagingData
{
    int staging;
    bool isCheack = false;
};
Q_DECLARE_METATYPE(StagingData);
class StandardTreatmentPage : public QWidget
{
    Q_OBJECT
public:
    StandardTreatmentPage(QWidget* parent = nullptr);
    ~StandardTreatmentPage();
    void initWidget();
    void initStandardTreatmentWidget();
public:
    QString currentPatientUid = "123";

private:
    std::unique_ptr<Ui::StandardTreatmentPage> ui;
    CCourseAllocationWidget* courseAllocationWidget;
    CStandardTreatmentService* standardTreatmentService;
    QString checkStyel = "\
        QPushButton: {\
            height: 56px;\
            color: rgb(255, 255, 255);\
            background-color: #162455;\
            border: 6px;\
            border-left-style: solid;\
            border-left-color: #00ffff;\
            border-right-style: solid;\
            border-right-color: rgba(255, 255, 255, 0);\
             }";

    QString unCheckStyle = "\
    QPushButton {\
        font-size: 16px;\
        font-family: 'Source Han Sans CN';\
        font-weight: bold;\
        height: 56px;\
        color: rgba(255, 255, 255, 0.8);\
        background-color: rgba(255, 255, 255, 0);\
        border: 6px;\
        border-radius: 0px;\
        border-left-style: solid;\
        border-left-color: rgba(255, 255, 255, 0);\
        border-right-style: solid;\
        border-right-color: rgba(255, 255, 255, 0);\
        margin-bottom: 1px;\
        text-align: left;\
        padding-left: 73px;\
    }\
    QPushButton:hover {\
        height: 56px;\
        color: rgb(255, 255, 255);\
        background-color: #162455;\
        border: 6px;\
        border-left-style: solid;\
        border-left-color: #00ffff;\
        border-right-style: solid;\
        border-right-color: rgba(255, 255, 255, 0);\
    }";
    QList<QPushButton*>buttons;
};

END_NX_NAMESPACE
#endif

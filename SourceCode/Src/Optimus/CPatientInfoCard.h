/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   November 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_PATIENT_INFO_CARD_H
#define __C_PATIENT_INFO_CARD_H
#include "Global.h"
#include <QWidget>
#include <memory>
#include "ui_CPatientInfoCard.h"


BEGIN_NX_NAMESPACE
struct Patient;
class CPatientAdminService;
class CPatientInfoCard : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER

public:
    CPatientInfoCard(CPatientAdminService* paService, const QString& patientUID, QWidget* parent = nullptr);
    ~CPatientInfoCard();

    void setSelected(const bool selected);
    bool selected()const { return isSelected; }

    QString patientUid() const;

    void updatePatientInfo() const;

private:
    void showAvatar() const;

signals:
    void signalPressed(QString uid);
    void signalDoubleClicked(QString patientUid);
    void signalJumpToTrainPage(QString patientUid);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    std::unique_ptr<Ui::CPatientInfoCard> ui;
    CPatientAdminService* service;
    QString curPatientUid;
    Patient* patientInfo;
    bool isSelected;
};

END_NX_NAMESPACE
#endif 

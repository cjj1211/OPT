/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __PATIENT_ADMIN_PAGE_H
#define __PATIENT_ADMIN_PAGE_H

#include "../Global.h"
#include "ui_CPatientAdminPage.h"
#include"../../NxEntity/Patient.h"
#include <QWidget>
#include <QMap>
#include <memory>

BEGIN_NX_NAMESPACE
class CPatientAdminService;
class CPatientInfoCard;
class CPatientInfo;

class CPatientAdminPage : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CPatientAdminPage(QWidget* parent = nullptr);
    ~CPatientAdminPage();
    
private:
    void showPage(int pageIndex = 1);
    
signals:
    void signalJumpToTrainPage(QString patientUid);

private slots:
    void slotFilterChanged(QString);
    void slotCreatePatient();
    void slotCreatePatientExit();
    void slotEditPatientExit(QString uid);
    void slotPatientDeleted();
    void slotImport();
    void slotExport();
    void slotSearch();
    void slotForward();
    void slotBack();
    void slotFastForward();
    void slotFastBack();
    void slotPatientCardPressed(QString uid);
    void slotPatientCardDoubleClicked(QString uid);
    void slotSelectAll();

private:
    std::unique_ptr<Ui::CPatientAdminPage> ui;
    CPatientAdminService* service;
    CPatientInfo* createPatienComonent;
    CPatientInfo* editPatientComponent;
    QMap<QString, CPatientInfoCard*> patientCardMap;
    QMap<int, QSpacerItem* > spacerMap;
    Patient currentPatient;
    QString filter;
};

END_NX_NAMESPACE
#endif

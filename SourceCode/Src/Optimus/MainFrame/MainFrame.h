/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __MAIN_FRAME_H
#define __MAIN_FRAME_H
#include "../Global.h"
#include "ui_MainFrame.h"

#include <QWidget>
#include <QMap>
#include <memory>
#include <QTimer>

namespace NX
{
    class CMainFrameService;
}

BEGIN_NX_NAMESPACE
    class Header;
class CPatientAdminPage;
class TrainPage;
class StatisticsPage;
class StandardTreatmentPage;
class CSystemSettings;
class MainFrame: public QWidget
{
    Q_OBJECT
public:
    MainFrame(QWidget* parent = nullptr);
    ~MainFrame() override;
    bool shutdown() const;
    void mouseMoveEvent();     //鼠标移动事件
    void resetTimer() const;

    void login();

public slots:
    void slotJumpToPage(PageID id);
    void slotFirstLogin();
    void slotShowLogin();
    void slotAdminLogin();

signals:
    void signalShowLogin();

  
private:
    std::unique_ptr<Ui::MainFrame> ui;
    CMainFrameService* service;
    Header* header;
    TrainPage* trainPage;
    CPatientAdminPage* patientAdminPage;
    StatisticsPage* statisticsPage;
    StandardTreatmentPage* standardTreatmentPage;
    CSystemSettings* systemSettings;

    /* 保存PageID 与 在stackWidget中pageIndex映射*/
    QMap<PageID, int> pageIdIndexMap;
    PageID currentPageID;
    int currentPageIndex;
    QTimer* timer;
    QPoint oldPos;   
    QTimer* mousetimer;

};
END_NX_NAMESPACE
#endif


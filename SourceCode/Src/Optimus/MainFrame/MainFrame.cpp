/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "MainFrame.h"

#include <utility>
#include "../Header/Header.h"
#include "../TrainPage/TrainPage.h"
#include "../PatientAdminPage/CPatientAdminPage.h"
#include "../StatisticsPage/StatisticsPage.h"
#include "../StandardTreatmentPage/StandardTreatmentPage.h"
#include "../CNxMessageBox.h"
#include "../SystemSettings/CSystemSettings.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "NxAppService/CMainFrameService.h"
#include "NxAppService/IAppService.h"

BEGIN_NX_NAMESPACE

MainFrame::MainFrame(QWidget* parent)
    :QWidget(parent)
    , service(new CMainFrameService())
    , trainPage(nullptr)
    , patientAdminPage(nullptr)
    , statisticsPage(nullptr)
    , standardTreatmentPage(nullptr)
    , systemSettings(nullptr)
{
    ui = std::make_unique<Ui::MainFrame>();
    ui->setupUi(this);

    header = new Header(this); // 通过Qt对象树管理，不需要主动删除资源
    ui->headerContainer->addWidget(header);

    connect(header, SIGNAL(signalJumpToPage(PageID)), this, SLOT(slotJumpToPage(PageID)));
    connect(header, &Header::signalLogout, this, [&]() {
            CNxMessageBox msgBox(tr("Logout"), tr("Program will Logout\n Click \"Confirm\" button to Logout program."));
            msgBox.exec();
            if (msgBox.isConfirmed())
            {
                emit signalShowLogin();
            }
            else
            {
                return;
            }
        });
    connect(header, &Header::signalShutdown, [&]() {
        shutdown();
        });
    setMouseTracking(true);
    //mousetimer = new QTimer(this);
    //connect(mousetimer, &QTimer::timeout, this, &MainFrame::mouseMoveEvent);
    //mousetimer->setInterval(1000);
    //// 启动计时器
    //mousetimer->start();
    oldPos = QCursor().pos();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainFrame::slotShowLogin);
    const auto logoutTime = systemConfig.GetSystemCfg().Common.LogoutTime * 1000 * 60;
    timer->setInterval(logoutTime);
    // 启动计时器
    timer->start();
}

MainFrame::~MainFrame() = default;

void  MainFrame::mouseMoveEvent()
{
    oldPos = QCursor().pos();
}

void MainFrame::resetTimer() const
{
    const auto logoutTime = systemConfig.GetSystemCfg().Common.LogoutTime * 1000 * 60;
    timer->setInterval(logoutTime);

    // 启动计时器
    timer->start();
}

void MainFrame::login()
{
    if (IAppService::getCurrentRole() == Role_Patient)
    {
        // 患者登录，当前患者与登录用户一致
        header->checkPageBtn(PAGE_TRAIN);
        slotJumpToPage(PAGE_TRAIN);
    }
    if (IAppService::getCurrentRole() ==Role_Admin)
    {
        header->checkPageBtn(PAGE_SYSTEM);
        slotJumpToPage(PAGE_SYSTEM);
    }
    else
    {
        const User user = service->getUserByIdentityFK(IAppService::getCurrentIdentityUid());
        const IdentityInfo identityInfo = service->getIdentityByUid(IAppService::getCurrentIdentityUid());
        header->setCurrentUserName(QString::fromStdString(user.UserName));
        header->showAvatar(identityInfo.Avatar);
        header->checkPageBtn(PAGE_PATIENT_ADMIN);
        slotJumpToPage(PAGE_PATIENT_ADMIN);
    }
}

bool MainFrame::shutdown() const
{
    if (trainPage && trainPage->isRecording()) {
        CNxMessageBox msgBox(tr("Shutdown"), tr("Program is recording\n Click \"Confirm\" button to close program."));
        msgBox.exec();
        if (msgBox.isConfirmed())
        {
            trainPage->stopRecord();
            qApp->exit();
        }
        else {
            return false;
        }
    }
    else {
        CNxMessageBox msgBox(tr("Shutdown"), tr("Program will closed\n Click \"Confirm\" button to close program."));
        msgBox.exec();
        if (msgBox.isConfirmed())
        {
            qApp->exit();
        }
        else {
            return false;
        }
    }
    return false;
}


void MainFrame::slotFirstLogin()
{
    CNxMessageBox msgBox(QString::fromLocal8Bit(""), tr("Please change your password"));
    msgBox.exec();
}

void MainFrame::slotShowLogin()
{
    if (oldPos == QCursor().pos() )
    {
      if(  trainPage &&trainPage->isRecording())
      {
             return; 
      }

          timer->stop();
          emit signalShowLogin();
    }
    else
    {
        oldPos = QCursor().pos();
    }
}

void MainFrame::slotAdminLogin()
{
    header->adminLogin();
}

void MainFrame::slotJumpToPage(PageID id)
{
    LOG_INFO("slotJumpToPage ..........................start");
    switch (currentPageID) {
    case PAGE_TRAIN:
        ui->trainPageContainer->removeWidget(trainPage);
        delete trainPage;
        trainPage = nullptr;
        pageIdIndexMap.remove(currentPageID);
        currentPageID = PAGE_NULL;
        currentPageIndex = 0;
        LOG_INFO("Exit train page.");
        break;
    case PAGE_PATIENT_ADMIN:
    case PAGE_STANDARD_TREATMENT:
    case PAGE_STATISTIC:
        // 当前只有训练页面，需要退出时释放资源
        break;
    default: 
        break;
    }

    switch (id)
    {
    case PAGE_PATIENT_ADMIN:
        if (!pageIdIndexMap.contains(id)) {
            patientAdminPage = new CPatientAdminPage(this);
            ui->patientAdminPageContainer->addWidget(patientAdminPage);
            connect(patientAdminPage, &CPatientAdminPage::signalJumpToTrainPage, [&](QString patientUid) {
                IAppService::setCurrentPatientUid(patientUid);
                header->checkPageBtn(PAGE_TRAIN);
                slotJumpToPage(PAGE_TRAIN);
                });
        }
        currentPageIndex = 1; // 根据UI文件获取
        currentPageID = PAGE_PATIENT_ADMIN;
        pageIdIndexMap.insert(PAGE_PATIENT_ADMIN, currentPageIndex);
        ui->stackedWidget->setCurrentIndex(currentPageIndex);
        LOG_INFO("Enter PatientAdminPage.");
        break;
    case PAGE_TRAIN:
        if (!pageIdIndexMap.contains(id)) {
            trainPage = new TrainPage(this);
            ui->trainPageContainer->addWidget(trainPage);
        }
        currentPageIndex = 0; // 根据UI文件获取
        currentPageID = PAGE_TRAIN;
        pageIdIndexMap.insert(PAGE_TRAIN, currentPageIndex);
        ui->stackedWidget->setCurrentIndex(currentPageIndex);
        LOG_INFO("Enter TrainPage.");
        break;
    case PAGE_STATISTIC:
        if (!pageIdIndexMap.contains(id)) {
            statisticsPage = new StatisticsPage(this);
            ui->statisticsPageContainer->addWidget(statisticsPage);
        }
        currentPageIndex = 2; // 根据UI文件获取
        currentPageID = PAGE_STATISTIC;
        pageIdIndexMap.insert(PAGE_STATISTIC, currentPageIndex);
        ui->stackedWidget->setCurrentIndex(currentPageIndex);
        LOG_INFO("Enter StatisticsPage.");
        break;
    case PAGE_STANDARD_TREATMENT:
        if (!pageIdIndexMap.contains(id)) {
            standardTreatmentPage = new StandardTreatmentPage(this);
            ui->standardTreatmentPageContainer->addWidget(standardTreatmentPage);
        }
        currentPageIndex = 3; // 根据UI文件获取
        currentPageID = PAGE_STANDARD_TREATMENT;
        pageIdIndexMap.insert(PAGE_STANDARD_TREATMENT, currentPageIndex);
        ui->stackedWidget->setCurrentIndex(currentPageIndex);
        LOG_INFO("Enter StandardTreatmentPage.");
        break;
    case PAGE_SYSTEM:
        if (!pageIdIndexMap.contains(id)) {
            systemSettings = new CSystemSettings(this);
            ui->systemSettingContainer->addWidget(systemSettings);
        }
        currentPageIndex = 4; // 根据UI文件获取
        currentPageID = PAGE_SYSTEM;
        pageIdIndexMap.insert(PAGE_SYSTEM, currentPageIndex);
        ui->stackedWidget->setCurrentIndex(currentPageIndex);
        LOG_INFO("Enter systemSettings.");
        break;
    default:
        break;
    }

    LOG_INFO("slotJumpToPage ..........................finished");
}

END_NX_NAMESPACE
#include "Optimus.h"
#include "MainFrame/MainFrame.h"
#include "Optimus/LoginPage/CLoginPage.h"
#include "Optimus/LoginPage/CLoadingScreen.h"
#include "CNxMessageBox.h"
#include <QCloseEvent>

BEGIN_NX_NAMESPACE
Optimus::Optimus(QWidget *parent)
    : QMainWindow(parent)
    , mainFrame(nullptr)
{
    translator = new QTranslator(this);

    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    ui.mainToolBar->hide();
    ui.statusBar->hide();
    loginPage = new CLoginPage(this);
    loadingScreen = new CLoadingScreen(this);
    loginPage->hide();
    ui.mainFrameContainer->addWidget(loadingScreen);

    mainFrame = new MainFrame(this);
    mainFrame->hide();
    connect(loginPage, &CLoginPage::signalFirstLogin, mainFrame, &MainFrame::slotFirstLogin);
    connect(loginPage, &CLoginPage::signalAdminLogin, mainFrame, &MainFrame::slotAdminLogin);
    connect(loginPage, &CLoginPage::signalShutdown, mainFrame, &MainFrame::shutdown);
    connect(mainFrame, &MainFrame::signalShowLogin, this, &Optimus::showLogin);
    connect(loginPage, &CLoginPage::signalLogin, this, &Optimus::showMainFrame);
    
    connect(loadingScreen, &CLoadingScreen::signalShowLogin, this, &Optimus::showLogin);
    this->showMaximized();
}

Optimus::~Optimus() = default;

void Optimus::showMainFrame(const bool isLogin)
{
    if (isLogin)
    {
        ui.mainFrameContainer->removeWidget(loginPage);
        loginPage->hide();
        mainFrame->login();
        ui.mainFrameContainer->addWidget(mainFrame);
        mainFrame->show();
        mainFrame->resetTimer();
       
    }
}

void Optimus::showLogin() const
{
    if(mainFrame)
    {
        ui.mainFrameContainer->removeWidget(mainFrame);
        mainFrame->hide();
    }
    ui.mainFrameContainer->removeWidget(loadingScreen);
    loadingScreen->hide();
    ui.mainFrameContainer->addWidget(loginPage);
    loginPage->show();
}

void Optimus::closeEvent(QCloseEvent* event)
{
    if (mainFrame && !mainFrame->shutdown()) {
        event->ignore();
    }
}

void Optimus::ChangeLanguage(const QString& language) const
{
    QString newLanguageFile;
    if (language.toLower() == "zh-cn")
    {
        newLanguageFile = ":/Optimus/Optimus_zh_CN.qm";    
    }
    else {
        newLanguageFile = "../config/Optimus_en_US.qm";
    }

    if (nullptr != translator)
    {
        qApp->removeTranslator(translator);
    }

    if (translator->load(newLanguageFile))
    {
        qApp->installTranslator(translator);
    }
}

void Optimus::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:  // NOLINT(clang-diagnostic-switch-enum)
        ui.retranslateUi(this);
        break;
    default:
        break;
    }
}

END_NX_NAMESPACE
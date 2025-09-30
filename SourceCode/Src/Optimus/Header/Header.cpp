/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "Header.h"
#include "NxSystemCfg/CSystemCfg.h"
#include "../CNxMessageBox.h"
#include <QMenu>
#include <QAction>
#include <QBitmap>
#include <QPainter>

#include "NxAppService/IAppService.h"

BEGIN_NX_NAMESPACE
    Header::Header(QWidget* parent)
    :QWidget(parent)
{
    ui = std::make_unique<Ui::Header>();
    ui->setupUi(this);
    // 创建一个QMenu，并添加一些QAction
    const auto menu = new QMenu(this);
    const QAction* logout = menu->addAction(tr("LogOut"));
    const QAction* shutdown = menu->addAction(tr("ShutDown"));
    menu->setStyleSheet(
        "QMenu { background-color: #162455; color: white; border: 1px solid #1FE9F4; border-radius: 1px; }"
        "QMenu::item { font-size: 14px; color: white; text-align: center; padding: 10px 30px; min-width: 30px; max-width: 30px; border:none; }"
        "QMenu::item:selected { background-color: #1890ff; color: white; border: none; }");
    
    ui->avatarLabel->hide();
    ui->userName->hide();
   /* ui->patientAdminBtn->hide();*/

    ui->trainBtn->setEnabled(false);
    ui->statisticsBtn->setEnabled(false);
    //ui->patientAdminBtn->setEnabled(false);
    ui->standardTreatmentBtn->setEnabled(true);
    ui->shutdownBtn->setMenu(menu);
    ui->shutdownBtn->setFlat(true);
    ui->settingsButton->setEnabled(true);
    connect(ui->trainBtn, SIGNAL(clicked()), this, SLOT(slotTrainBtnClick()));
    connect(ui->patientAdminBtn, SIGNAL(clicked()), this, SLOT(slotPatientAdminBtnClick()));
    connect(ui->statisticsBtn, SIGNAL(clicked()), this, SLOT(slotStatisticsPageClick()));
    connect(ui->standardTreatmentBtn, SIGNAL(clicked()), this, SLOT(slotStandardTreatmentClick()));
    connect(ui->settingsButton, SIGNAL(clicked()), this, SLOT(slotSettingsClick()));
    connect(shutdown, SIGNAL(triggered()), this, SLOT(slotShutdown()));
    connect(logout, SIGNAL(triggered()), this, SLOT(slotLogout()));
    const auto ver = QString::fromUtf8(systemConfig.GetSystemCfg().Common.Version.c_str());
    //auto ver = "";
    ui->versionLabel->setText(ver);
 
}

Header::~Header() = default;

void Header::slotTrainBtnClick()
{
    emit signalJumpToPage(PAGE_TRAIN);
    checkPageBtn(PAGE_TRAIN);
}

void Header::slotPatientAdminBtnClick()
{
    emit signalJumpToPage(PAGE_PATIENT_ADMIN);
    checkPageBtn(PAGE_PATIENT_ADMIN);
}

void Header::slotStatisticsPageClick()
{
    emit signalJumpToPage(PAGE_STATISTIC);
    checkPageBtn(PAGE_STATISTIC);
}

void Header::slotStandardTreatmentClick()
{
    emit signalJumpToPage(PAGE_STANDARD_TREATMENT);
    checkPageBtn(PAGE_STANDARD_TREATMENT);
}

void Header::checkPageBtn(const PageID id) const
{
    const auto pageBtns = {ui->trainBtn, ui->patientAdminBtn, ui->statisticsBtn, ui->standardTreatmentBtn,ui->settingsButton};

    for (const auto btn : pageBtns) {
        btn->setChecked(false);
    }

    switch (id)
    {
    case PAGE_NULL:
        break;
    case PAGE_PATIENT_ADMIN:
        ui->trainBtn->setEnabled(false);
        ui->patientAdminBtn->setChecked(true);
        if (IAppService::getCurrentRole() != Role_Admin)
        {
            ui->settingsButton->setEnabled(false);
        }
        else
        {
            ui->settingsButton->setEnabled(true);
            ui->patientAdminBtn->setEnabled(false);
  
        }
     

        break;
    case PAGE_TRAIN:
        ui->trainBtn->setEnabled(true);
        ui->trainBtn->setChecked(true);
        if (IAppService::getCurrentRole() == Role_Patient)
        {
            ui->patientAdminBtn->hide();
            ui->settingsButton->hide();
        }
        if (IAppService::getCurrentRole() !=Role_Admin)
        {
            ui->settingsButton->setEnabled(false);
        }
        else
        {
            ui->settingsButton->setEnabled(true);
            ui->patientAdminBtn->setEnabled(true);
        }

        break;
    case PAGE_STATISTIC:
        if (IAppService::getCurrentRole() != Role_Patient)
        {
            ui->trainBtn->setEnabled(false);
        }
        if (IAppService::getCurrentRole() != Role_Admin)
        {
            ui->settingsButton->setEnabled(false);
        }
        else
        {
            ui->settingsButton->setEnabled(true);
           
        }

        break;
    case PAGE_STANDARD_TREATMENT:
        if (IAppService::getCurrentRole() != Role_Patient)
        {
            ui->trainBtn->setEnabled(false);
        }
        if (IAppService::getCurrentRole() != Role_Admin)
        {
            ui->settingsButton->setEnabled(false);
        }
        else
        {
            ui->settingsButton->setEnabled(true);
      
        }
        ui->standardTreatmentBtn->setChecked(true);
        break;
    case PAGE_SYSTEM:
        if (IAppService::getCurrentRole() != Role_Patient)
        {
            ui->trainBtn->setEnabled(false);
        }
        if (IAppService::getCurrentRole() != Role_Admin)
        {
            ui->settingsButton->setEnabled(false);
        }
        else
        {
            ui->settingsButton->setEnabled(true);
            ui->settingsButton->setChecked(true);
            
        }
   
        break;
    }
}

void  Header::slotShutdown()
{
    emit signalShutdown();
}

void Header::slotSettingsClick()
{
    emit signalJumpToPage(PAGE_SYSTEM);
    checkPageBtn(PAGE_SYSTEM);
}

void Header::adminLogin() const
{
    ui->settingsButton->setEnabled(true);
    ui->settingsButton->setChecked(true);
   /* ui->patientAdminBtn->setCheckable(false);*/
    ui->patientAdminBtn->setEnabled(false);
}

void Header::setCurrentUserName(const QString& userName)
{
    currentUserName = userName;
    ui->userName->setText(currentUserName);
    ui->patientAdminBtn->show();
    ui->avatarLabel->show();
    ui->userName->show();
}

void Header::showAvatar(const std::vector<char>& avatar) const
{
    if (!avatar.empty()) {
        QByteArray ba;
        for (const char i : avatar)
        {
            ba.append(i);
        }

        QImage img;
        img.loadFromData(ba, "jpg");
        ba.clear();
        auto showImg = QPixmap::fromImage(img.scaled(ui->avatarLabel->width(), ui->avatarLabel->height(), Qt::KeepAspectRatio));

        const auto radius = ui->avatarLabel->width() / 2;
        const QSize size(ui->avatarLabel->width(), ui->avatarLabel->height());
        QBitmap mask(size);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
        painter.setBrush(QColor(220, 0, 0));
        painter.drawEllipse(QPoint(radius, radius), radius, radius);
        showImg.setMask(mask);
        ui->avatarLabel->setPixmap(showImg);
    }
    else
    {
        ui->avatarLabel->setPixmap(QPixmap(":/Optimus/images/common/avatar 30x30.png"));
    }
}

void  Header::slotLogout()
{
    emit signalLogout();
}
void  Header::userLogin() const
{
    ui->settingsButton->setEnabled(false);
}
END_NX_NAMESPACE



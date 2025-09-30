/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "StandardTreatmentPage.h"
#include "CCourseAllocationWidget.h"
#include "NxAppService/CStandardTreatmentService.h"
#include <QComboBox>

BEGIN_NX_NAMESPACE
StandardTreatmentPage::StandardTreatmentPage(QWidget* parent)
    :QWidget(parent),
    courseAllocationWidget(new CCourseAllocationWidget()),
    standardTreatmentService(new CStandardTreatmentService(currentPatientUid))
{
    ui = std::make_unique<Ui::StandardTreatmentPage>();
    ui->setupUi(this);
    qRegisterMetaType<StagingData>("StagingData");
    StagingData customData;

    ui->stage1->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));   customData.staging = 0;
    ui->stage1->setProperty("customData", QVariant::fromValue(customData));
    ui->verticalLayout_3->addWidget(courseAllocationWidget);
    ui->stage2->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 1;
    ui->stage2->setProperty("customData", QVariant::fromValue(customData));
    ui->stage3->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 2;
    ui->stage3->setProperty("customData", QVariant::fromValue(customData));
    ui->stage4->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 3;
    ui->stage4->setProperty("customData", QVariant::fromValue(customData));
    ui->stage5->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 4;
    ui->stage5->setProperty("customData", QVariant::fromValue(customData));
    ui->stage6->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 5;
    ui->stage6->setProperty("customData", QVariant::fromValue(customData));
    ui->stage7->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 6;
    ui->stage7->setProperty("customData", QVariant::fromValue(customData));
    ui->stage8->setIcon(QIcon(":/Optimus/images/trainPage/stage.svg"));
    customData.staging = 7;
    ui->stage8->setProperty("customData", QVariant::fromValue(customData));
    connect(ui->stage1, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage2, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage3, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage4, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage5, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage6, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage7, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->stage8, &QPushButton::clicked, this, &StandardTreatmentPage::initStandardTreatmentWidget);
    connect(ui->comboBox, &QComboBox::currentTextChanged, this, &StandardTreatmentPage::initWidget);
    buttons.push_back(ui->stage1);
    buttons.push_back(ui->stage2);
    buttons.push_back(ui->stage3);
    buttons.push_back(ui->stage4);
    buttons.push_back(ui->stage5);
    buttons.push_back(ui->stage6);
    buttons.push_back(ui->stage7);
    buttons.push_back(ui->stage8);
    ui->comboBox->setCurrentIndex(1);
    ui->comboBox->setCurrentIndex(0);
    ui->stage1->clicked();
    ui->stage1->setChecked(true);

}

StandardTreatmentPage::~StandardTreatmentPage()
{
}

void StandardTreatmentPage::initWidget()
{
    switch (ui->comboBox->currentIndex())
    {
    case 0:
        ui->stage1->setText(QString::fromLocal8Bit("1期"));
        ui->stage2->setText(QString::fromLocal8Bit("2期A"));
        ui->stage3->setText(QString::fromLocal8Bit("2期B"));
        ui->stage4->setText(QString::fromLocal8Bit("3期"));
        ui->stage5->setText(QString::fromLocal8Bit("4期A"));
        ui->stage6->setText(QString::fromLocal8Bit("4期B"));
        ui->stage7->setText(QString::fromLocal8Bit("5期"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(true);
        ui->stage5->setVisible(true);
        ui->stage6->setVisible(true);
        ui->stage7->setVisible(true);
        ui->stage8->setVisible(false);

        break;
    case 1:
        ui->stage1->setText(QString::fromLocal8Bit("Ⅰ级"));
        ui->stage2->setText(QString::fromLocal8Bit("Ⅱ级"));
        ui->stage3->setText(QString::fromLocal8Bit("Ⅲ级"));
        ui->stage4->setText(QString::fromLocal8Bit("Ⅳ级"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(true);
        ui->stage5->setVisible(false);
        ui->stage6->setVisible(false);
        ui->stage7->setVisible(false);
        ui->stage8->setVisible(false);
        break;
    case 2:
        ui->stage1->setText(QString::fromLocal8Bit("Ⅰ级"));
        ui->stage2->setText(QString::fromLocal8Bit("Ⅱ级"));
        ui->stage3->setText(QString::fromLocal8Bit("Ⅲ级"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(false);
        ui->stage5->setVisible(false);
        ui->stage6->setVisible(false);
        ui->stage7->setVisible(false);
        ui->stage8->setVisible(false);
        break;
    case 3:
        ui->stage1->setText(QString::fromLocal8Bit("A级"));
        ui->stage2->setText(QString::fromLocal8Bit("B级"));
        ui->stage3->setText(QString::fromLocal8Bit("C级"));
        ui->stage4->setText(QString::fromLocal8Bit("D级"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(true);
        ui->stage5->setVisible(false);
        ui->stage6->setVisible(false);
        ui->stage7->setVisible(false);
        ui->stage8->setVisible(false);
        break;
    case 4:
        ui->stage1->setText(QString::fromLocal8Bit("0期"));
        ui->stage2->setText(QString::fromLocal8Bit("1期"));
        ui->stage3->setText(QString::fromLocal8Bit("1.5期"));
        ui->stage4->setText(QString::fromLocal8Bit("2期"));
        ui->stage5->setText(QString::fromLocal8Bit("2.5期"));
        ui->stage6->setText(QString::fromLocal8Bit("3期"));
        ui->stage7->setText(QString::fromLocal8Bit("4期"));
        ui->stage8->setText(QString::fromLocal8Bit("5期"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(true);
        ui->stage5->setVisible(true);
        ui->stage6->setVisible(true);
        ui->stage7->setVisible(true);
        ui->stage8->setVisible(true);
        break;
    case 5:
        ui->stage1->setText(QString::fromLocal8Bit("1级"));
        ui->stage2->setText(QString::fromLocal8Bit("2级"));
        ui->stage3->setText(QString::fromLocal8Bit("3级"));
        ui->stage4->setText(QString::fromLocal8Bit("4级"));
        ui->stage5->setText(QString::fromLocal8Bit("5级"));
        ui->stage6->setText(QString::fromLocal8Bit("6级"));
        ui->stage7->setText(QString::fromLocal8Bit("7级"));
        ui->stage8->setText(QString::fromLocal8Bit("8级"));
        ui->stage1->setVisible(true);
        ui->stage2->setVisible(true);
        ui->stage3->setVisible(true);
        ui->stage4->setVisible(true);
        ui->stage5->setVisible(true);
        ui->stage6->setVisible(true);
        ui->stage7->setVisible(true);
        ui->stage8->setVisible(true);

        break;
    default:
        break;
    }
    ui->stage1->clicked();
    ui->stage1->setChecked(true);
}

void StandardTreatmentPage::initStandardTreatmentWidget()
{
    if (QPushButton* thisbutton = qobject_cast<QPushButton*>(sender()))
    {
        for (auto button : buttons)
        {
            if (button != thisbutton)
            {
                auto cusdata = button->property("customData").value<StagingData>();
                cusdata.isCheack = false;
                button->setProperty("customData", QVariant::fromValue(cusdata));
                button->setStyleSheet(unCheckStyle);
            }

        }
        auto data = thisbutton->property("customData").value<StagingData>();
        data.isCheack = true;
        thisbutton->setProperty("customData", QVariant::fromValue(data));
        thisbutton->setStyleSheet(checkStyel);
        courseAllocationWidget->setPatientInfo("123", ui->comboBox->currentIndex(), data.staging);
        courseAllocationWidget->initWidget();
        thisbutton->setChecked(true);
    }
}

END_NX_NAMESPACE
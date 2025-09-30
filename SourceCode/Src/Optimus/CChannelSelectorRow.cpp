#include "CChannelSelectorRow.h"
#include "CFilterSettingDialog.h"
#include "NxAppService/COscillographService.h"

BEGIN_NX_NAMESPACE

CChannelSelectorRow::CChannelSelectorRow(COscillographService* oscillographService, ChannelPlan* plan, const bool showSignalTypeAndSetting, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CChannelSelectorRow())
    , service(oscillographService)
    , channelPlan(plan)
    , filterSettingDialog(nullptr)
{
    ui->setupUi(this);
    ui->portCheckBox->setText(QString::fromStdString(plan->Indexs));
    ui->portCheckBox->setChecked(plan->isEnabled());
    ui->signalTypeCombo->setCurrentIndex(plan->getSignalType());
    //ui->signalTypeCombo->setCurrentText(QString::fromStdString(plan->SignalType));
    //ui->signalTypeCombo->setEnabled(false);

    connect(ui->settingLabel, SIGNAL(clicked()), SLOT(slotSettingBtnClicked()));
    connect(ui->portCheckBox, SIGNAL(toggled(bool)), SLOT(slotCheckedFromUi(bool)));
    connect(ui->signalTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSignalTypeChanged(int)));

    if (!showSignalTypeAndSetting)
    {
        ui->settingLabel->hide();
        ui->signalTypeCombo->hide();
    }
}

CChannelSelectorRow::~CChannelSelectorRow() = default;

void CChannelSelectorRow::slotSettingBtnClicked()
{
    if (nullptr == filterSettingDialog)
    {
        filterSettingDialog = new CFilterSettingDialog(service, channelPlan,this);
        connect(filterSettingDialog, SIGNAL(signalSampleRateChanged()), SIGNAL(signalSampleRateChanged()));
    }
    filterSettingDialog->show();
}

void CChannelSelectorRow::slotCheckedFromUi(bool enabled)
{
    channelPlan->setEnabled(enabled);
    service->updateChannelPlan(*channelPlan);
    emit signalChannelPlanToggled(QString::fromStdString(channelPlan->UID), enabled);
}

void CChannelSelectorRow::slotChannelPlanToggled(bool enabled)
{
    if (ui->portCheckBox->isChecked() != enabled)
    {
        channelPlan->setEnabled(enabled);
        service->updateChannelPlan(*channelPlan);
        ui->portCheckBox->setChecked(enabled);
    }
}

bool CChannelSelectorRow::isChecked()
{
    return ui->portCheckBox->isChecked();
}

void CChannelSelectorRow::updateEnabledStatus()
{
    auto newPlan = service->getChannelPlanByUid(QString::fromStdString(channelPlan->UID));
    channelPlan->setEnabled(newPlan.isEnabled());
    bool enabled = channelPlan->isEnabled();
    bool isChecked = ui->portCheckBox->isChecked();
    if (ui->portCheckBox->isChecked() != enabled) {
        ui->portCheckBox->setChecked(channelPlan->isEnabled());
    }
}


void CChannelSelectorRow::slotSignalTypeChanged(int index)
{
    channelPlan->setSignalType(static_cast<NX_SignalType>(index));
    service->updateChannelPlan(*channelPlan); // TODO 需要通知server切换信号类型
}
END_NX_NAMESPACE


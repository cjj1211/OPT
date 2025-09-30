#include "CFilterSettingDialog.h"
#include "NxAppService/COscillographService.h"
#include "NxEntity/ChannelPlan.h"
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

#include "CChannelSelectorCard.h"

BEGIN_NX_NAMESPACE
    const int sampleRateList[] = { 500, 800, 1000,1250,1500, 2000, 2500,3000,3333,4000,5000,6250, 8000,10000, 20000, 25000, 30000};

CFilterSettingDialog::CFilterSettingDialog(COscillographService* oscillographService, ChannelPlan* plan, QWidget* parent)
    : CDragableDialog(parent)
    , ui(new Ui::CFilterSettingDialog())
    , service(oscillographService)
    , channelPlan(plan)
    , isSampleRateChanged(false)
    , isFilterChanged(false)
{
    ui->setupUi(this);

    this->setWindowModality(Qt::NonModal);

    // 设置无边框和背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::FramelessWindowHint;
    setWindowFlags(flags);

    // 设置阴影效果
    const auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(Qt::black);
    shadowEffect->setBlurRadius(15);
    this->setGraphicsEffect(shadowEffect);

    // 设置滤波参数初始值
    const auto firstPlan = service->getChannelPlans()[0];
    ui->lowCutSpinBox->setMaximum(firstPlan.SampleRate / 2 - 1);
    ui->lowCutSpinBox->setToolTip(tr("Software low cut filter range at ") + "0.1 ~ " + QString::number(firstPlan.SampleRate / 2, 'f', 0) + "Hz");
    ui->highCutSpinBox->setMaximum(firstPlan.SampleRate / 2 - 1);
    ui->highCutSpinBox->setToolTip(tr("Software high cut filter range at ") + "0.1 ~ " + QString::number(firstPlan.SampleRate / 2, 'f', 0) + "Hz");

    ui->hwLcf->setMinimum(0.1);
    ui->hwLcf->setMaximum(500);
    ui->hwLcf->setToolTip(tr("Hardware low cut filter range at ") + "0.1 ~ 500 Hz");
    ui->hwHcf->setMinimum(100);
    ui->hwHcf->setMaximum(firstPlan.SampleRate / 2);
    ui->hwHcf->setToolTip(tr("Hardware high cut filter range at ") + "500 Hz~" + QString::number(firstPlan.SampleRate / 2, 'f', 0)+" Hz");
    ui->dspCutoff->setMinimum(0.000004857 * firstPlan.SampleRate);
    ui->dspCutoff->setMaximum(0.1103 * firstPlan.SampleRate);
    ui->dspCutoff->setToolTip(tr("Dsp cutoff range at ") + QString::number(0.000004857 * firstPlan.SampleRate, 'f', 2) + " Hz~" +
        QString::number(0.1103 * firstPlan.SampleRate, 'f', 0) + " Hz");

    ui->areaNum->setText(CChannelSelectorCard::getAreaName(*channelPlan));
    ui->portChar->setText(QString::fromStdString(channelPlan->Indexs));
    if(service->isPlayback())
    {
        ui->sampleRateCombo->setEnabled(false);
        ui->hwHcf->setEnabled(false);
        ui->hwLcf->setEnabled(false);
        ui->dspCheck->setEnabled(false);
        ui->dspCutoff->setEnabled(false);
    }
    ui->sampleRateCombo->setCurrentText(QString::number(channelPlan->SampleRate ) + " Hz");
    if ("50" == channelPlan->Notch) {
        ui->notchCombo->setCurrentText("50 Hz");
    }
    else if ("60" == channelPlan->Notch) {
        ui->notchCombo->setCurrentText("60 Hz");
    }
    else {
        ui->notchCombo->setCurrentText(tr("None"));
    }
    
    ui->highCutSpinBox->setValue(channelPlan->HighCut);
    ui->lowCutSpinBox->setValue(channelPlan->LowCut);
    ui->lowCutCheck->setChecked(channelPlan->EnableLowCut);
    ui->HighCutCheck->setChecked(channelPlan->EnableHighCut);
    ui->hwLcf->setValue(firstPlan.HwLowCut);
    ui->hwHcf->setValue(firstPlan.HwHighCut);
    ui->dspCutoff->setValue(firstPlan.DspCutoff);
    ui->dspCheck->setChecked(firstPlan.EnableDSP);

    connect(ui->sampleRateCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSampleRateChanged(int)));
    connect(ui->notchCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotFilterChanged()));
    connect(ui->lowCutSpinBox, SIGNAL(valueChanged(double)), SLOT(slotFilterChanged()));
    connect(ui->highCutSpinBox, SIGNAL(valueChanged(double)), SLOT(slotFilterChanged()));
    connect(ui->hwLcf, SIGNAL(valueChanged(double)), SLOT(slotFilterChanged()));
    connect(ui->hwHcf, SIGNAL(valueChanged(double)), SLOT(slotFilterChanged()));
    connect(ui->lowCutCheck, SIGNAL(toggled(bool)), SLOT(slotFilterChanged()));
    connect(ui->HighCutCheck, SIGNAL(toggled(bool)), SLOT(slotFilterChanged()));
    connect(ui->dspCutoff, SIGNAL(valueChanged(double)), SLOT(slotFilterChanged()));
    connect(ui->confirmBtn, SIGNAL(clicked()), SLOT(slotConfirm()));
}

CFilterSettingDialog::~CFilterSettingDialog() = default;

// 每次显示时，刷新页面数据
void CFilterSettingDialog::showEvent(QShowEvent* event)
{
    ui->sampleRateCombo->setCurrentText(QString::number(channelPlan->SampleRate ) + " Hz");
    if ("50" == channelPlan->Notch) {
        ui->notchCombo->setCurrentText("50 Hz");
    }
    else if ("60" == channelPlan->Notch) {
        ui->notchCombo->setCurrentText("60 Hz");
    }
    else {
        ui->notchCombo->setCurrentText(tr("None"));
    }
    ui->highCutSpinBox->setValue(channelPlan->HighCut);
    ui->lowCutSpinBox->setValue(channelPlan->LowCut);
    ui->lowCutCheck->setChecked(channelPlan->EnableLowCut);
    ui->HighCutCheck->setChecked(channelPlan->EnableHighCut);
    const auto firstPlan = service->getChannelPlans()[0];
    ui->hwLcf->setValue(firstPlan.HwLowCut);
    ui->hwHcf->setValue(firstPlan.HwHighCut);
    ui->dspCutoff->setValue(firstPlan.DspCutoff);
    ui->dspCheck->setChecked(firstPlan.EnableDSP);

    isSampleRateChanged = false;
    isFilterChanged = false;
}

void CFilterSettingDialog::slotSampleRateChanged(int index)
{
    isSampleRateChanged = true;
}

void CFilterSettingDialog::slotFilterChanged()
{
    isFilterChanged = true;
}


void CFilterSettingDialog::slotConfirm()
{
    auto firstPlan = service->getChannelPlans()[0];
    if (isFilterChanged)
    {
        const auto n = QString::fromLocal8Bit("无");
        const std::string notchList[] = { n.toStdString(), "50", "60"};

        channelPlan->Notch = notchList[ui->notchCombo->currentIndex()];
        channelPlan->LowCut = ui->lowCutSpinBox->value();
        channelPlan->HighCut = ui->highCutSpinBox->value();
        channelPlan->HwLowCut = ui->hwLcf->value();
        channelPlan->HwHighCut = ui->hwHcf->value();
        channelPlan->EnableLowCut = ui->lowCutCheck->isChecked();
        channelPlan->EnableHighCut = ui->HighCutCheck->isChecked();
        channelPlan->EnableDSP = ui->dspCheck->isChecked();
        channelPlan->DspCutoff = ui->dspCutoff->value();

        firstPlan.HwLowCut = channelPlan->HwLowCut;
        firstPlan.HwHighCut = channelPlan->HwHighCut;
        firstPlan.DspCutoff = channelPlan->DspCutoff;
        firstPlan.EnableDSP = channelPlan->EnableDSP;
        
        service->updateChannelPlan(*channelPlan);
        if (firstPlan.UID != channelPlan->UID) {
            service->updateChannelPlan(firstPlan);
        }
        service->updateFilterParam(*channelPlan);
    }
    if(isSampleRateChanged)
    {
        firstPlan.SampleRate = sampleRateList[ui->sampleRateCombo->currentIndex()];
        channelPlan->SampleRate = firstPlan.SampleRate;
        service->updateChannelPlan(firstPlan);
        service->updateChannelPlan(*channelPlan);
        emit signalSampleRateChanged();
    }
    close();
}

END_NX_NAMESPACE


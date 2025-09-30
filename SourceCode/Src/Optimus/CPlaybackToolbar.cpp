#include "CPlaybackToolbar.h"

BEGIN_NX_NAMESPACE

CPlaybackToolbar::CPlaybackToolbar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CPlaybackToolbarClass())
    , sliderPressed(false)
{
    ui->setupUi(this);
    connect(ui->playBtn, SIGNAL(toggled(bool)), SLOT(slotPlayBtnToggled(bool)));
    connect(ui->impedanceBtn, SIGNAL(clicked()), SIGNAL(signalImpedance()));
    connect(ui->spikeScopeBtn, SIGNAL(clicked()), SIGNAL(signalSpikeScope()));
    connect(ui->exportBtn, SIGNAL(clicked()), SIGNAL(signalExport()));
    connect(ui->playbackSlider, SIGNAL(valueChanged(int)), SLOT(slotSliderChanged(const int)));
    connect(ui->playbackSlider, SIGNAL(sliderReleased()), SLOT(slotJumpTo()));
    connect(ui->playbackSlider, SIGNAL(sliderPressed()), SLOT(slotSliderPressed()));
    connect(ui->backBtn, SIGNAL(clicked()), SIGNAL(signalBack()));
}

CPlaybackToolbar::~CPlaybackToolbar()
{
    delete ui;
}

void CPlaybackToolbar::setTimeLength(const int timeLenInSecs) const
{
    const auto hour = timeLenInSecs / 3600;
    const auto minute = (timeLenInSecs % 3600) / 60;
    const auto sec = timeLenInSecs % 60;
    ui->timeLenLabel->setText(QString("%1:%2:%3")
        .arg(hour, 3, 10, QLatin1Char('0'))
        .arg(minute, 2, 10, QLatin1Char('0'))
        .arg(sec, 2, 10, QLatin1Char('0')));

    ui->playbackSlider->setMaximum(timeLenInSecs);
}

void CPlaybackToolbar::jumpByEvent(const int timeLenInSecs)
{
    if(ui->playBtn->isChecked())
    {
        ui->playBtn->setChecked(false);
    }
    ui->playbackSlider->setValue(timeLenInSecs);

    LOG(std::format("Jump by event, jump to time: {}", timeLenInSecs).c_str());
    emit signalJumpTo(timeLenInSecs);
}

void CPlaybackToolbar::slotUpdateRuntime(const uint64_t runtimeInSecs)
{
    auto hour = runtimeInSecs / 3600;
    auto minute = (runtimeInSecs % 3600) / 60;
    auto sec = runtimeInSecs % 60;

    ui->runtimeLabel->setText(QString("%1:%2:%3")
        .arg(hour, 3, 10, QLatin1Char('0'))
        .arg(minute, 2, 10, QLatin1Char('0'))
        .arg(sec, 2, 10, QLatin1Char('0')));
    if (!sliderPressed)
    {
        ui->playbackSlider->setValue(runtimeInSecs);
    }
}

void CPlaybackToolbar::slotJumpTo()
{
    if (ui->playBtn->isChecked())
    {
        ui->playBtn->setChecked(false);
    }
    const int runtimeInSecs = ui->playbackSlider->value();
    emit signalJumpTo(runtimeInSecs);
    sliderPressed = false;
}

void CPlaybackToolbar::slotSliderChanged(int runtimeInSecs)
{
    auto hour = runtimeInSecs / 3600;
    auto minute = (runtimeInSecs % 3600) / 60;
    auto sec = runtimeInSecs % 60;
    ui->runtimeLabel->setText(QString("%1:%2:%3")
        .arg(hour, 3, 10, QLatin1Char('0'))
        .arg(minute, 2, 10, QLatin1Char('0'))
        .arg(sec, 2, 10, QLatin1Char('0')));
}

void CPlaybackToolbar::slotServerStoped()
{
    ui->playBtn->toggle();
    ui->runtimeLabel->setText(ui->timeLenLabel->text());
    ui->playbackSlider->setValue(ui->playbackSlider->maximum());
}

void CPlaybackToolbar::slotSliderPressed()
{
   sliderPressed = true;
}

void CPlaybackToolbar::slotPlayBtnToggled(bool isRun)
{
    if (isRun && ui->playbackSlider->value() == ui->playbackSlider->maximum()) {
        emit signalJumpTo(0);
        emit signalPlay(isRun);
    }
    else {
        emit signalPlay(isRun);
    }
}

END_NX_NAMESPACE

#include "CDetectToolbar.h"

BEGIN_NX_NAMESPACE

CDetectToolbar::CDetectToolbar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CDetectToolbarClass())
{
    ui->setupUi(this);

    connect(ui->startBtn, SIGNAL(clicked(bool)), SLOT(slotRun(bool)));
    connect(ui->recordBtn, SIGNAL(clicked(bool)), SLOT(slotRecord(bool)));
    connect(ui->spikeBtn, SIGNAL(clicked()), SIGNAL(signalSpikeScope()));
    connect(ui->impedanceBtn, SIGNAL(clicked()), SIGNAL(signalImpedance()));
    connect(ui->channelMapBtn, SIGNAL(clicked()), SIGNAL(signalChannelMap()));
}

CDetectToolbar::~CDetectToolbar()
{
    delete ui;
}

void CDetectToolbar::slotRun(bool isRun)
{
    if (isRun) {
        ui->recordBtn->setChecked(false);
    }
    emit signalRun(isRun);
}

void CDetectToolbar::slotRecord(bool isStartRecord)
{
    if (isStartRecord)
    {
        ui->startBtn->setChecked(false);
    }
    
    emit signalRecord(isStartRecord);
}

void CDetectToolbar::slotUpdateRuntime(const uint64_t runTimeInSecs)
{
    int runHour = runTimeInSecs / 3600;
    int runMinutes = (runTimeInSecs % 3600) / 60;
    int runSecs = runTimeInSecs % 60;
    QString runTime = QString("%1:%2:%3")
        .arg(runHour, 3, 10, QLatin1Char('0'))
        .arg(runMinutes, 2, 10, QLatin1Char('0'))
        .arg(runSecs, 2, 10, QLatin1Char('0'));
    ui->timestamp->setText(runTime);
}


END_NX_NAMESPACE
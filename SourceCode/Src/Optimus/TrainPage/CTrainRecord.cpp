#include "CTrainRecord.h"
#include "../CTrainRecordComponent.h"
#include "CDetect.h"
#include "NxAppService/CTreatmentRecordService.h"

BEGIN_NX_NAMESPACE

CTrainRecord::CTrainRecord(const QString& currentPatientUid, QWidget* parent)
    :QWidget(parent)
    , ui(new Ui::TrainRecord())
    , patientUid(currentPatientUid)
    , service(new CTreatmentRecordService(currentPatientUid, this))
    , playback(nullptr)
{
    ui->setupUi(this);
    record = new CTrainRecordComponent(service, this);
    ui->recordPageLayout->addWidget(record);

    connect(record, SIGNAL(signalJumpToPlayback(const QString&)), SLOT(slotJumpToPlayback(const QString&)));
}

CTrainRecord::~CTrainRecord()
{
    delete service;
}

void CTrainRecord::slotJumpToPlayback(const QString& recordUid) {
    if (nullptr == playback)
    {
        playback = new CDetect(patientUid, recordUid,this);
        ui->playbackPageLayout->addWidget(playback);
        connect(playback, SIGNAL(signalPlaybackExit()), SLOT(slotExitPlayback()));
        playback->initOscillograph();
    }
    ui->stackedWidget->setCurrentWidget(ui->playbackPage);
}

void CTrainRecord::slotExitPlayback()
{
    if (nullptr != playback)
    {
        ui->playbackPageLayout->removeWidget(playback);
        delete playback;
        playback = nullptr;
    }
    
    ui->stackedWidget->setCurrentWidget(ui->recordPage);
}
END_NX_NAMESPACE

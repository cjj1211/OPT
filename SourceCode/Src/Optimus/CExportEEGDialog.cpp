#include "CExportEEGDialog.h"

#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QThread>
#include <qsettings.h>

#include "CProgressBarDialog.h"

#include "NxAppService/COscillographService.h"

const QString kExportFileTypes[] = { "rhd", "edf", "mat" };
const QString kTargetPathKey = "CExportEEGDialog_TargetPath";

CExportEEGDialog::CExportEEGDialog(COscillographService* service_, QWidget *parent)
    : CDragableDialog(parent)
    , ui(new Ui::CExportEEGDialog())
    , service(service_)
{
    ui->setupUi(this);

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

    const QSettings settings;
    const auto targetPath = settings.value(kTargetPathKey).toString();
    ui->targetPath->setText(targetPath);

    exportTimer.setInterval(100);

    connect(ui->openDir, SIGNAL(clicked()), this, SLOT(slotOpenDialog()));
    connect(ui->exportBtn, SIGNAL(clicked()), this, SLOT(slotExport()));
    connect(&exportTimer, SIGNAL(timeout()), this, SLOT(slotUpdateProgress()));
    connect(ui->targetPath, SIGNAL(textChanged(QString)), this, SLOT(slotTargetPathChanged(QString)));
}

CExportEEGDialog::~CExportEEGDialog() = default;

void CExportEEGDialog::slotOpenDialog()
{
    const auto selectDir = QFileDialog::getExistingDirectory();
    if (!selectDir.isEmpty())
    {
        ui->targetPath->setText(selectDir);
    }
    ui->targetPath->show();
    qApp->processEvents();
}

void CExportEEGDialog::slotExport()
{
    const auto destDir = ui->targetPath->text();
    const auto exportFileType = kExportFileTypes[ui->exportType->currentIndex()];
    if(!service->exportData(destDir, exportFileType))
    {
        LOG_ERR("Create export task fail.");
    }

    this->setVisible(false);
    progressBar = new CProgressBarDialog();
    progressBar->slotUpdateProgress(0);
    progressBar->slotUpdateContentText(tr("Exporting EEG signal data, it will be automatically turned off when it is done"));
    progressBar->show();
    progressBar->raise();
    update();
    QThread::msleep(100);
    exportTimer.start();
}

void CExportEEGDialog::slotUpdateProgress()
{
    const auto progress = service->getExportProgress();
    progressBar->slotUpdateProgress(progress);
    if(progress == 100)
    {
        exportTimer.stop();

        QThread::msleep(500);
        progressBar->close();
        progressBar->deleteLater();
        this->close();
    }
    update();
}

void CExportEEGDialog::slotTargetPathChanged(QString text)
{

    if(QDir(text).exists(text))
    {
        ui->exportBtn->setEnabled(true);
        QSettings settings;
        settings.setValue(kTargetPathKey, text);
    }
    else
    {
        ui->exportBtn->setEnabled(false);
    }
}

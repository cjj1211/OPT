#include "CTrainRecordComponent.h"
#include "NxAppService/CTreatmentRecordService.h"
#include "NxEntity/TreatmentRecord.h"
#include <QTableWidgetItem>
#include <QDatetime>
#include <algorithm>

BEGIN_NX_NAMESPACE

CTrainRecordComponent::CTrainRecordComponent(CTreatmentRecordService* treatmentRecordService, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CTrainRecordComponent())
    , service(treatmentRecordService)
{
    ui->setupUi(this);

    QPalette p = ui->recordTable->palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    ui->recordTable->setPalette(p);

    const QStringList headerList = { tr("Index"), tr("Train Time"), tr("Duration"),  
        tr("Train Type"), tr("Score"), tr("Result"), tr("Difficulty"),
        tr("Success Times"), tr("Fail Times"),tr("File Type"),tr("Note"),tr("Data Path")};
    
    
    ui->recordTable->setColumnCount(headerList.count());
    ui->recordTable->setHorizontalHeaderLabels(headerList);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(0, 50);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(1, 150);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(2, 100);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(3, 100);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(4, 50);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(5, 70);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(6, 70);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(7, 80);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(8, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(8, 80);
    ui->recordTable->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Interactive);
    ui->recordTable->setColumnWidth(9, 80);
    ui->recordTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    records = QVector<TreatmentRecord>::fromStdVector(service->getTreatments());
    if (records.isEmpty()) {
        ui->analysisBtn->setEnabled(false);
        ui->deleteBtn->setEnabled(false);
        ui->playbackBtn->setEnabled(false);
        ui->reportBtn->setEnabled(false);
    }

    std::sort(records.begin(), records.end(), [](TreatmentRecord& a, TreatmentRecord& b) {
        const auto aStart = QDateTime::fromString(QString::fromStdString(a.StartDatetime), "yyyy-MM-dd hh:mm:ss");
        const auto bStart = QDateTime::fromString(QString::fromStdString(b.StartDatetime), "yyyy-MM-dd hh:mm:ss");
        return aStart > bStart;
        });
    
    for (auto& record : records)
    {
        auto startDatetime = QDateTime::fromString(QString::fromStdString(record.StartDatetime), "yyyy-MM-dd hh:mm:ss").toString("yyyy-MM-dd hh:mm");
        const auto rowCnt = ui->recordTable->rowCount();

        ui->recordTable->insertRow(rowCnt);
        ui->recordTable->setItem(rowCnt, 0, new QTableWidgetItem(QString::number(rowCnt + 1)));
        ui->recordTable->item(rowCnt, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 1, new QTableWidgetItem(startDatetime));
        ui->recordTable->item(rowCnt, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 2, new QTableWidgetItem(record.getTimeLenStr()));
        ui->recordTable->item(rowCnt, 2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 3, new QTableWidgetItem(record.getTrainTypeStr()));
        ui->recordTable->item(rowCnt, 3)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 4, new QTableWidgetItem(QString::number(record.Score)));
        ui->recordTable->item(rowCnt, 4)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 5, new QTableWidgetItem(record.IsSuccess ? tr("Success"): tr("Fail")));
        ui->recordTable->item(rowCnt, 5)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 6, new QTableWidgetItem(QString::number(record.Difficult)));
        ui->recordTable->item(rowCnt, 6)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 7, new QTableWidgetItem(QString::number(record.SuccessTimes)));
        ui->recordTable->item(rowCnt, 7)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 8, new QTableWidgetItem(QString::number(record.FailTimes)));
        ui->recordTable->item(rowCnt, 8)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 9, new QTableWidgetItem(QString::fromStdString(record.SaveFileType)));
        ui->recordTable->item(rowCnt, 9)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 10, new QTableWidgetItem(QString::fromStdString(record.Note)));
        ui->recordTable->item(rowCnt, 10)->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->recordTable->setItem(rowCnt, 11, new QTableWidgetItem(QString::fromStdString(record.TrainDataPath)));
        ui->recordTable->item(rowCnt, 11)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
    // 默认选中首行
    if (!records.empty())
    {
        ui->recordTable->selectRow(0);
    }

    connect(ui->deleteBtn, SIGNAL(clicked()), SLOT(slotDeleteClicked()));
    connect(ui->recordTable, SIGNAL(itemSelectionChanged()), SLOT(slotRowSelected()));
    connect(ui->playbackBtn, SIGNAL(clicked()), SLOT(slotPlaybackClicked()));
    connect(ui->recordTable, SIGNAL(cellChanged(int, int)), SLOT(slotCellChanged(int, int)));
}

CTrainRecordComponent::~CTrainRecordComponent() = default;

void CTrainRecordComponent::slotRowSelected()
{
    // TODO
}

void CTrainRecordComponent::slotPlaybackClicked()
{
    const auto selectedRow = ui->recordTable->currentRow();
    emit signalJumpToPlayback(QString::fromStdString(records[selectedRow].UID));
}

void CTrainRecordComponent::slotRecordSelected()
{
}

void CTrainRecordComponent::slotAnalysisClicked()
{
}

void CTrainRecordComponent::slotDeleteClicked()
{
    const auto selectedRow = ui->recordTable->currentRow();
    if (service->deleteTreatmentByUid(QString::fromStdString(records[selectedRow].UID)))
    {
        ui->recordTable->removeRow(selectedRow);
        if (!records.empty())
        {
            ui->recordTable->selectRow(0);
        }
        records.remove(selectedRow);
    }
    else
    {
        LOG_ERR("Remove treatment record fail, uid:" + records[selectedRow].UID);
    }
}

void CTrainRecordComponent::slotCellChanged(int row, int col) 
{
    const QString note = ui->recordTable->item(row, col)->text();
    records[row].Note = note.toStdString();
    service->updateTreatmentRecord(records[row]);
}

END_NX_NAMESPACE

#include "CImpedanceWidget.h"
#include "CChannelSelectorCard.h"
#include "CColorGradientWidget.h"
#include "CProgressDialog.h"
#include "NxElectrodeMap/CElectrodePreview.h"
#include "NxAppService/COscillographService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxWaveformWidget/CSignalSources.h"
#include <QMap>
#include <QtConcurrent/QtConcurrent>
#include <QTextStream>

BEGIN_NX_NAMESPACE

CImpedanceWidget::CImpedanceWidget(COscillographService* sc, bool canMeasure, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CImpedanceWidget())
    , service(sc)
    , colorGradient(new CColorGradientWidget(GT_IMPEDANCE, tr("Electrode Impedance"), this))
    , channelMap(new CElectrodePreview(this))
    , progressDialog(nullptr)
    , detectorInfos("")
{
    ui->setupUi(this);
    ui->colorGradientContainer->addWidget(colorGradient);
    ui->channelMapContainer->addWidget(channelMap);

    if (!canMeasure)
    {
        ui->measueBtn->hide();
    }
    
    connect(ui->measueBtn, SIGNAL(clicked()), this, SLOT(slotMeasureImpedance()));
    connect(this, SIGNAL(signalMeasureImpedanceDone()), this, SLOT(slotUpdateChannelMap()));
    connect(this, SIGNAL(signalUpdateMeasureStatus(bool, int)), this, SLOT(slotUpdateMeasureStatus(bool, int)));
    connect(colorGradient, SIGNAL(signalGradientChanged()), this, SLOT(slotUpdateChannelMap()));
    connect(ui->backBtn, SIGNAL(clicked()), this, SIGNAL(signalClosed()));
    connect(ui->impedanceList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), 
        SLOT(slotCurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));
    initChannelSelectorCard();
}

CImpedanceWidget::~CImpedanceWidget()
{
    isMeasuring = false;
    updateMeasureStatusThread.waitForFinished();
}

void CImpedanceWidget::initChannelSelectorCard(){
    auto channelPlans = service->getChannelPlans();
    QVector<std::string> areaNums;

    for (auto& plan : channelPlans)
    {
        if (!areaNums.contains(plan.AreaNum))
        {
            areaNums.append(plan.AreaNum);
        }
        channelMap->populateTabWidget({ QString::fromStdString(plan.Indexs), QString::fromStdString(plan.DetectorType)}); // TODO
        enabledPort.insert(QString::fromStdString(plan.Indexs), plan.isEnabled());
    }

    std::ranges::sort(areaNums);
    for (const auto& areaNum : areaNums)
    {
        QVector<ChannelPlan*> plansInArea;
        for (auto& plan : channelPlans)
        {
            if (plan.AreaNum == areaNum)
            {
                plansInArea.append(new ChannelPlan(plan));
            }
        }

        auto card = new CChannelSelectorCard(service, plansInArea, false, this);
        channelSelectorCardVec.append(card);
        ui->channelSelectCardContainer->addWidget(card);
        
        connect(card, SIGNAL(signalChannelPlanToggled(const QString&, bool)), SLOT(slotChannelPlanToggled(QString&, bool)));
    }

    updateImpedanceList();
}

void CImpedanceWidget::slotMeasureImpedance() {
    isMeasuring = true;
    if (nullptr == progressDialog)
    {
        progressDialog = new CProgressDialog(Qt::WindowModal, this);
    }

    detectorInfos = "";
    const auto plans = service->getChannelPlans();
    QString spliter = "";
    for (auto plan : plans)
    {
        if (plan.isEnabled())
        {
            detectorInfos.append(spliter + QString::fromStdString(plan.Indexs + "(" + plan.DetectorType + ")") );
            spliter = ",";
        }
    }

    progressDialog->setContent(detectorInfos + tr("Measuring impedance, please wait."));
    progressDialog->show();

    QtConcurrent::run([&]() {
        QStringList enabledPortNames;
        for (auto name : enabledPort.keys())
        {
            if (enabledPort[name])
            {
                enabledPortNames.append(name);
            }
        }

        QVector<ImpedanceInfo> impedanceInfos;
        service->getImpedanceInfos(enabledPortNames, impedanceInfos);
        for (auto info : impedanceInfos)
        {
            const auto channel = signalManager.channelByName(QString::fromStdString(info.nativeChannelName));
            if (nullptr != channel)
            {
                channel->setImpednanceValided(true);
                channel->setImpedanceMagnitude(info.magnitude);
                channel->setImpedancePhase(info.phase);
            }
        }

        emit signalMeasureImpedanceDone();
    });

    updateMeasureStatusThread = QtConcurrent::run([&]() {
        bool isCompleted = false;
        int progress = 0;
        QThread::msleep(500);
        while (!isCompleted && isMeasuring) {
            service->getImpedanceMeasureStatus(isCompleted, progress);
            emit signalUpdateMeasureStatus(isCompleted, progress);
            QThread::msleep(100);
        }
        isMeasuring = false;
    });
}


void CImpedanceWidget::slotUpdateMeasureStatus(bool isCompleted, int progress)
{
    if (progressDialog) {
        if (isCompleted) {
            progressDialog->close();
            isMeasuring = false;
        }
        else {
            progressDialog->setContent(detectorInfos + tr("Measuring impedance, please wait.") + QString::number(progress) + "%");
        }
    }
}


void CImpedanceWidget::slotUpdateChannelMap()
{
    QMap<QString, QColor> channelColors;
    QMap<QString, QString> impedanceValue;
    for (int i = 0; i < signalManager.numChannels(); ++i)
    {
        const auto channel = signalManager.channelByIndex(i);
        if (channel->isImpedanceValided())
        {
            auto color = colorGradient->getColor(std::log(channel->getImpedanceMagnitude() / 1000.0) / std::log(10000000.0 / 1000.0));
            channelColors.insert(channel->getCustomName(), color);
            impedanceValue.insert(channel->getCustomName(), channel->getCustomName() + " " + QString::number(channel->getImpedanceMagnitude() / 1000.0));
        }
        else
        {
            //channelColors.insert(channel->getNativeName(), QColor("#000000"));
        }
    }

    channelMap->setButtonStyle(channelColors);
    channelMap->setImpedanceValue(impedanceValue);

    if (nullptr != progressDialog)
    {
        progressDialog->close();
    }

    updateImpedanceList();
}

void CImpedanceWidget::slotChannelPlanToggled( QString& uid, bool enabled)
{
    const auto plan = service->getChannelPlanByUid(uid);
    if (enabledPort.contains(QString::fromStdString(plan.Indexs)))
    {
        enabledPort[QString::fromStdString(plan.Indexs)] = enabled;
    }
    emit signalChannelPlanToggled(uid, enabled);
}

void CImpedanceWidget::slotCurrentItemChanged( QListWidgetItem* current, QListWidgetItem* previous) {
    if (current == nullptr)
    {
        return;
    }
    const auto filePath =  service->getImpedanceDataPath() + "/" + impedanceHisMap[current->text()] ;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMap<QString, QColor> channelColors;
        QMap<QString, QString> impedanceValue;
        QTextStream in(&file); 
        QString line = in.readLine();// 首行表头
        while (!line.isNull())//字符串有内容
        {
            bool ok = true;
            line = in.readLine();//循环读取下行
            if (line.isEmpty())
            {
                continue;
            }
            auto nativeName = line.split(",")[0];
            const double magnitude = line.split(",")[4].toDouble(&ok);
            double phase = line.split(",")[5].toDouble(&ok);

            QString magnitudeStr = "";
            if (magnitude >= 1000000) {
                magnitudeStr = QString::number(magnitude / 1000000.0,'f',2) + QString::fromLocal8Bit("MΩ");
            }
            else if (magnitude >= 1000) {
                magnitudeStr = QString::number(magnitude / 1000.0, 'f', 2) + QString::fromLocal8Bit("kΩ");
            }
            else {
                magnitudeStr = QString::number(magnitude) + QString::fromLocal8Bit("Ω");
            }

            const auto channel = signalManager.channelByName(nativeName);
            if (channel != nullptr)
            {
                auto color = colorGradient->getColor(log(magnitude /1000.0) / log(10000000 / 1000));
                channelColors.insert(channel->getCustomName(), color);
                impedanceValue.insert(channel->getCustomName(), channel->getCustomName() + " " + magnitudeStr);
            }
        }

        channelMap->setButtonStyle(channelColors);
        channelMap->setImpedanceValue(impedanceValue);
    }
}

void CImpedanceWidget::updateImpedanceList()
{
    const QDir impedanceDir(service->getImpedanceDataPath());
    auto list = impedanceDir.entryList(QStringList() << "*.csv", QDir::Files, QDir::Time);
    impedanceHisMap.clear();
    ui->impedanceList->clear();

    for (const QString& impedanceFile : list)
    {
        auto itemText = filename2itemName(impedanceFile);
        impedanceHisMap.insert(itemText, impedanceFile);
        ui->impedanceList->addItem(itemText);
    }

    if (ui->impedanceList->count() > 0)
    {
        ui->impedanceList->setCurrentRow(0);
    }
}

QString CImpedanceWidget::filename2itemName(QString filename)
{
    const auto yearStr = filename.remove(".csv").split("-")[0].right(2);
    const auto monthStr = filename.remove(".csv").split("-")[1];
    const auto dateStr = filename.remove(".csv").split("-")[1];

    const auto timeStr = filename.remove(".csv").split(" ")[1].replace("-", ":");
    return yearStr + "-" + monthStr + "-" + dateStr + " " + timeStr;
}


void CImpedanceWidget::updateEnabledStatus()
{
    for (const auto card : channelSelectorCardVec) {
        card->updateEnabledStatus();
    }
}
END_NX_NAMESPACE


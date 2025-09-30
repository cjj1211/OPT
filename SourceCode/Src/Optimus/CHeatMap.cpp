#include "CHeatMap.h"
#include "ui_CHeatMap.h"
#include "CColorGradientWidget.h"
#include "NxAppService/COscillographService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/ChannelName.h"
#include "NxElectrodeMap/CElectrodePreview.h"
#include "NxWaveformWidget/CSignalSources.h"
#include <QStringList>
#include <algorithm>
#include <QtConcurrent/QtConcurrent>

BEGIN_NX_NAMESPACE

CHeatMap::CHeatMap(COscillographService* sc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CHeatMap())
    , service(sc)
    , colorGradient(new CColorGradientWidget(GT_HEAT,tr("Electrode Heat"), this))
    , channelMap(new CElectrodePreview(this))
    , isLoadingHeatScales(false)
{
    ui->setupUi(this);
    ui->colorGradientContainer->addWidget(colorGradient);
    ui->channelMapContainer->addWidget(channelMap);
    ui->warning->setVisible(false);

    connect(channelMap, &CElectrodePreview::signalShowCurrentChannelName, this, &CHeatMap::showCurrentChannelName, Qt::DirectConnection);
    connect(colorGradient, SIGNAL(signalGradientChanged()), this, SLOT(slotUpdateChannelMap()));
    connect(ui->backBtn, SIGNAL(clicked()), this, SIGNAL(signalClosed()));
    connect(ui->searchBtn, SIGNAL(clicked()), this, SLOT(searchChannelMap()));
    connect(this, SIGNAL(signalHeatValueLoaded()), this, SLOT(slotUpdateChannelMap()), Qt::QueuedConnection);

    initChannelMap();
    setHeatScaleChannels();
    startMeasureHeat();
}

CHeatMap::~CHeatMap()
{
    isLoadingHeatScales = false;
    loadHeatScaleThread.waitForFinished();
}

void CHeatMap::initChannelMap()
{
    auto channelPlans = service->getChannelPlans();
    QVector<std::string> areaNums;
    for (auto& plan : channelPlans)
    {
        if (!areaNums.contains(plan.AreaNum))
        {
            areaNums.append(plan.AreaNum);
        }
        groupNameMap.insert(QString::fromStdString(plan.Indexs), QString::fromStdString(plan.DetectorType));
        channelMap->populateTabWidget({ QString::fromStdString(plan.Indexs), QString::fromStdString(plan.DetectorType) });
    }
    if (channelPlans.size() > 0)
    {
        currentGroup = QString::fromStdString(channelPlans[0].Indexs);
    }
}

void CHeatMap::setHeatScaleChannels()
{
    QStringList curGroupChannelNames;
    auto channelNames = signalManager.getDisplayListAmplifiers();
    for (auto& channelName : channelNames)
    {
        auto channel = signalManager.channelByName(channelName);
        if (channel->getGroupName() == currentGroup)
        {
            curGroupChannelNames.append(channel->getNativeName());
        }
    }

    service->setHeatScaleChannels(curGroupChannelNames);
}

void CHeatMap::startMeasureHeat()
{
    isLoadingHeatScales = true;
    loadHeatScaleThread = QtConcurrent::run([=]() {
        QElapsedTimer timer;
        
        while (isLoadingHeatScales) {
            if (isVisible())
            {
                timer.restart();
                QMap<QString, float> heatScales;
                service->getHeatScales(heatScales);

                for (const auto& name : heatScales.keys())
                {
                    if(const auto channel = signalManager.channelByName(name))
                    {
                        channel->setHeatValid(true);
                        channel->setHeatValue(heatScales[name]);
                    }
                }
                emit signalHeatValueLoaded();
                auto t = timer.elapsed();
                if (t < 1000)
                {
                    QThread::currentThread()->msleep(1000 - t);
                }
            }
        }
        });
}

void CHeatMap::slotUpdateChannelMap()
{
    QMap<QString, double> heatScales;
    double max = -65535;
    double min = 65535;
    auto channelNames = signalManager.getDisplayListAmplifiers();
    for (auto& nativeChannelName : channelNames)
    {
        auto channel = signalManager.channelByName(nativeChannelName);
        if (channel->isEnabled() && channel->getGroupName() == currentGroup && channel->getHeatValid()) {
            auto value = channel->getHeatValue();
            heatScales.insert(channel->getCustomName(), value);
            max = max > value ? max : value;
            min = min > value ? value : min;
        }
    }

    auto range = max - min;
    QMap<QString, QColor> channelColors;
    QMap<QString, QString> heatVule;
    for (const auto& customName : heatScales.keys())
    {
        auto normaledVal = (heatScales[customName] - min) / (max - min);
        auto color = colorGradient->getColor(normaledVal);
        channelColors.insert(customName, color);
        heatVule.insert(customName, customName + "\n" + QString::number(normaledVal, 'f', 2));
    }

    channelMap->setButtonStyle(channelColors);
    channelMap->setImpedanceValue(heatVule);
}

void CHeatMap::searchChannelMap()
{
    auto customName = ui->channelNameEdit->text();
    auto plans = service->getChannelPlans();
    auto curPlan = std::find_if(plans.begin(), plans.end(), [&](ChannelPlan p) { return p.Indexs == currentGroup.toStdString(); });
    auto position = service->getChannelPosition(QString::fromStdString(curPlan[0].UID), customName);
    if (position == QPoint(-1, -1))
    {
        auto number = customName.mid(2, customName.length());

        if (channelMap->searchChannelMap(customName.mid(0, 1), QString::number(number.toInt())))
        {
            ui->warning->setVisible(false);
            return;
        }
        else
        {
         
            ui->warning->setVisible(true);
            return;
        }
    }
    else
    {
        auto channelName = service->getChannelByChannelPosition(QString::fromStdString(curPlan[0].UID), position.x(), position.y(), QString::fromStdString(curPlan[0].Indexs));
        if (channelName.UID == "")
        {
            ui->warning->setVisible(true);
            return;
        }

        auto channelPlan = service->getChannelPlanByUid(QString::fromStdString(channelName.ChannelPlanFK));
        if (channelPlan.UID == "")
        {
            ui->warning->setVisible(true);
            return;
        }
        else
        {
            QPair  <int, int>posi;
            posi.first = channelName.ChannelCoordX;
            posi.second = channelName.ChannelCoordY;
            if (channelMap->searchChannelMap(customName.mid(0, 1), posi))
            {
                ui->warning->setVisible(false);
            }

            else
            {
                ui->warning->setVisible(true);
            }
        }
    }
}

void CHeatMap::showCurrentChannelName(int currentButtonX, int currentButtonY, QString channelNumber, QString currentTabIndex)
{
    auto groupName = groupNameMap.keys().at(currentTabIndex.toInt());
    if (groupName != currentGroup)
    {
        currentGroup = groupName;
        setHeatScaleChannels();
    }
    auto plans = service->getChannelPlans();
    auto curPlan = std::find_if(plans.begin(), plans.end(), [&](ChannelPlan p) { return p.Indexs == currentGroup.toStdString(); });
    auto channelName = service->getChannelByChannelPosition(QString::fromStdString(curPlan[0].UID), currentButtonX, currentButtonY, QString::fromStdString(curPlan[0].Indexs));
    auto channelPlan = service->getChannelPlanByUid(QString::fromStdString(channelName.ChannelPlanFK));
    if (channelName.UID != "")
    {
        ui->channelNameEdit->setText(QString::fromStdString(channelName.CustomName));
    }
    else
    {
        ui->channelNameEdit->setText(groupName + QString("-%1").arg(channelNumber.toInt(), 3, 10, QLatin1Char('0')));
    }
    ui->warning->setVisible(false);
}

END_NX_NAMESPACE
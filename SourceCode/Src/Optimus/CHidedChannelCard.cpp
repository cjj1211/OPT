#include "CHidedChannelCard.h"
#include "NxAppService/COscillographService.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/ChannelName.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

BEGIN_NX_NAMESPACE
CHidedChannelCard::CHidedChannelCard(COscillographService* oscillographService, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CHidedChannelCard())
    , service(oscillographService)
{
    ui->setupUi(this);
    auto plans = service->getChannelPlans();
    for (auto& plan : plans) {
        auto channelNumbers = plan.getDisabledChannelNumbers();
        auto channelNamesCfg = service->getChannelNamesByPlan(QString::fromStdString(plan.UID));
        std::sort(channelNamesCfg.begin(), channelNamesCfg.end(), [](ChannelName a, ChannelName b) {
            auto na = QString::fromStdString(a.ChannelNumber).toInt();
            auto nb = QString::fromStdString(b.ChannelNumber).toInt();
            return  na < nb;
            });
        for (auto nameCfg : channelNamesCfg) {
            if (channelNumbers.contains(QString::fromStdString(nameCfg.ChannelNumber))) {
                addHidedChannelWidget(QString::fromStdString(nameCfg.CustomName));
            }
        }
    }
}

CHidedChannelCard::~CHidedChannelCard()
{
}

void CHidedChannelCard::hideChannel(const QString& customWavename)
{
    addHidedChannelWidget(customWavename);
}

void CHidedChannelCard::addHidedChannelWidget(const QString& customWavename) {

    if (hidedChannelMap.contains(customWavename)) {
        hidedChannelMap[customWavename]->setEnabled(true);
        hidedChannelMap[customWavename]->setChecked(false);
        hidedChannelMap[customWavename]->show();
        showChannelAnimations[customWavename]->start();
    }
    else {
        auto checkBox = new QCheckBox(this);
        checkBox->setToolTip(tr("Check to show this channel's signal"));
        checkBox->setText(customWavename);
        hidedChannelMap.insert(customWavename, checkBox);
        if (hidedChannelMap.count() % 2 == 1)
        {
            ui->leftContainer->addWidget(checkBox);
        }
        else
        {
            ui->rightContainer->addWidget(checkBox);
        }
        auto opacityEffect = new QGraphicsOpacityEffect(checkBox);
        opacityEffect->setOpacity(1.0);
        checkBox->setGraphicsEffect(opacityEffect);

        QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity", checkBox);
        animation->setEasingCurve(QEasingCurve::Linear);
        animation->setDuration(600);
        animation->setStartValue(1);
        animation->setEndValue(0);
        hidedChannelAnimations.insert(customWavename, animation);

        QPropertyAnimation* showAnimation = new QPropertyAnimation(opacityEffect, "opacity", checkBox);
        showAnimation->setEasingCurve(QEasingCurve::Linear);
        showAnimation->setDuration(10);
        showAnimation->setStartValue(0);
        showAnimation->setEndValue(1);
        showChannelAnimations.insert(customWavename, showAnimation);

        connect(animation, &QPropertyAnimation::finished, checkBox , &QCheckBox::hide);
        connect(checkBox, SIGNAL(toggled(bool)), SLOT(slotHidedWaveChecked()));
    }
    hidedCustomNames.insert(customWavename);
    
}

void CHidedChannelCard::showChannel(const QString& customWavename)
{
    removeHidedChannelWidget(customWavename);
}

void CHidedChannelCard::removeHidedChannelWidget(const QString& customWavename) {
    if (hidedChannelMap.contains(customWavename))
    {
        hidedChannelMap[customWavename]->setEnabled(false);
        hidedChannelAnimations[customWavename]->start();
        hidedCustomNames.remove(customWavename);
    }
}

void CHidedChannelCard::slotHidedWaveChecked() {
    for (auto& customWavename : hidedCustomNames)
    {
        if (hidedChannelMap[customWavename]->isChecked())
        {
            hidedChannelMap[customWavename]->setEnabled(false);
            emit signalShowChannel(customWavename);

            hidedChannelAnimations[customWavename]->start();
            hidedCustomNames.remove(customWavename);
            break;
        }
    }
    saveHidedChannels();
}

QSet<QString> CHidedChannelCard::getHidedWavenaems()
{
    return hidedCustomNames;
}

void CHidedChannelCard::saveHidedChannels()
{
    auto plans = service->getChannelPlans();
    for (auto& plan : plans) {
        QSet<QString> hidedChannelNumbers;
        auto nameCfgs = service->getChannelNamesByPlan(QString::fromStdString(plan.UID));
        for (auto& nameCfg : nameCfgs) {
            if (hidedCustomNames.contains(QString::fromStdString(nameCfg.CustomName))) {
                hidedChannelNumbers.insert(QString::fromStdString(nameCfg.ChannelNumber));
            }
        }
        plan.setDisabledChannelNumbers(hidedChannelNumbers);
        service->updateChannelPlan(plan);
    }
} 
END_NX_NAMESPACE



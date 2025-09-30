#include "CChannelSelectorCard.h"
#include "NxAppService/COscillographService.h"

#include "NxBrainAreaWidget/AtlasConfiguration.h"

#include "NxSystemCfg/CSystemCfg.h"

BEGIN_NX_NAMESPACE

const QString kHemisphereNames[] = { "Left Hemisphere","Right Hemisphere" };
const QString kHemisphereNames_CN[] = { QString::fromLocal8Bit("вСдт"), QString::fromLocal8Bit("срдт") };

CChannelSelectorCard::CChannelSelectorCard(COscillographService* oscillographService, QVector<ChannelPlan*> plans, const bool showSignalTypeAndSetting, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CChannelSelectorCard())
    , service(oscillographService)
    , channelPlans(plans)
{
    ui->setupUi(this);
    ui->areaCheckBox->setText(getAreaName(*plans[0]));
    ui->label->hide();
    
    bool isNotAllDisabled = false;
    for (const auto& plan : plans)
    {
        auto row = new CChannelSelectorRow(service, plan, showSignalTypeAndSetting, this);
        rowsMap.insert(plan->UID, row);
        ui->rowLayout->addWidget(row);
        connect(row, SIGNAL(signalChannelPlanToggled(const QString&, bool)), SLOT(slotChannelPlanToggled(const QString&, bool)));
        connect(row, SIGNAL(signalSampleRateChanged()), this, SIGNAL(signalSampleRateChanged()));
        isNotAllDisabled = isNotAllDisabled || plan->isEnabled();
    }
    ui->areaCheckBox->setChecked(isNotAllDisabled);
    connect(ui->areaCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotAreaCheckBoxChanged(bool)));
}

CChannelSelectorCard::~CChannelSelectorCard() = default;

void CChannelSelectorCard::slotAreaCheckBoxChanged(bool enabled)
{
    for (auto row : rowsMap)
    {
        if (row->isChecked() != enabled)
        {
            row->slotChannelPlanToggled(enabled);
        }
    }
}

void CChannelSelectorCard::slotChannelPlanToggled(const QString& uid, bool enabled)
{
    bool isAllChecked = true;
    bool isAllUnchecked = true;
    for (auto row : rowsMap)
    {
        if (!row->isChecked())
        {
            isAllChecked = false;
            break;
        }
    }
    if (isAllChecked)
    {
        ui->areaCheckBox->setChecked(true);
    }
    else
    {
        for (auto row : rowsMap)
        {
            if (row->isChecked())
            {
                isAllUnchecked = false;
                break;
            }
        }
        if (isAllUnchecked)
        {
            ui->areaCheckBox->setChecked(false);
        }
    }

    emit signalChannelPlanToggled(uid, enabled);
}

QString CChannelSelectorCard::getAreaName(const ChannelPlan& plan)
{
    auto hemisphere = brainAtlasCfg.getAtlas().LeftHemisphere;
    if (plan.Position == kHemisphereNames[1].toStdString())
    {
        hemisphere = brainAtlasCfg.getAtlas().RightHemisphere;
    }

    for (auto lobe : hemisphere)
    {
        if (lobe.Name == plan.Area)
        {
            for (auto area : lobe.Areas)
            {
                if (area.Name == plan.AreaNum)
                {
                    if (systemConfig.GetSystemCfg().Common.Language == "zh-CN")
                    {
                        return QString::fromStdString(area.ChineseName);
                    }
                    else
                    {
                        return QString::fromStdString(area.Name);
                    }
                }
            }

        }
    }
    return {};
}

void CChannelSelectorCard::updateEnabledStatus()
{
    bool isNotAllDisabled = false;
    for (auto& plan : channelPlans)
    {
        auto newPlan = service->getChannelPlanByUid(QString::fromStdString(plan->UID));
        plan->setEnabled(newPlan.isEnabled());
        isNotAllDisabled = isNotAllDisabled || plan->isEnabled();
    }
    if (isNotAllDisabled != ui->areaCheckBox->isChecked()) {
        ui->areaCheckBox->setChecked(isNotAllDisabled);
    }
    for (auto row : rowsMap) {
        row->updateEnabledStatus();
    }
}
END_NX_NAMESPACE



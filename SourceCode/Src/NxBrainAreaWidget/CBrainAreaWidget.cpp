#include "CBrainAreaWidget.h"
#include "ui_CBrainAreaWidget.h"
#include "CBrainAreaUtility.h"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkNamedColors.h>
#include <vtkOpenGLRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <QElapsedTimer>
#include <QDebug>
#include <QMovie>
#include <QTimer>
#include <QVector3D>
#include <QDebug>
#include <QStandardItemModel>

#include <QtConcurrent/QtConcurrent>

#include "AtlasConfiguration.h"

#include "NxSystemCfg/CSystemCfg.h"

BEGIN_NX_NAMESPACE
    QMap<QString, int> DestrieuxAtlasR = {
    {"Corpus",12100},
    {"G_and_S_frontomargin",12101},
    {"G_and_S_occipital_inf",12102},
    {"G_and_S_paracentral",12103},
    {"G_and_S_subcentral",12104},
    {"G_and_S_transv_frontopol",12105},
    {"G_and_S_cingul-Ant",12106},
    {"G_and_S_cingul-Mid-Ant",12107},
    {"G_and_S_cingul-Mid-Post",12108},
    {"G_cingul-Post-dorsal",12109},
        {"G_cuneus",12111},
    {"G_front_inf-Opercular",12112},
    {"G_front_inf-Orbital",12113},
    {"G_front_inf-Triangul",12114},
    {"G_front_middle",12115},
    {"G_front_sup",12116},
    {"G_Ins_lg_and_S_cent_ins",12117},
    {"G_insular_short",12118},
    {"G_and_S_cingul-Mid-Post",12108},
    {"G_occipital_middle",12119},
    {"G_occipital_sup",12120},
    {"G_oc-temp_med-Lingual",12122},
    {"G_oc-temp_med-Parahip",12123},
    {"G_orbital",12124},
    {"G_pariet_inf-Angular",12125},
    {"G_pariet_inf-Supramar",12126},
    {"G_parietal_sup",12127},
    {"G_postcentral",12128},
    {"G_precentral",12129},
    {"G_precunneus",12130},
    {"G_rectus",12131},
    {"G_subcallosal",12132},
    {"G_temp_sup-G_T_transv",12133},
    {"G_temp_sup-Lateral",12134},
    {"G_temp_sup-Plan_polar",12135},
    {"G_temp_sup-Plan_tempo",12136},
    {"G_temporal_inf",12137},
    {"G_temporal_middle",12138},
    {"Lat_Fis-ant-Horizont",12139},
    {"Lat_Fis-ant-Vertical",12140},
    {"Lat_Fis-post",12141},
    {"Pole_occipital",12143},
    {"Pole_temporal",12144},
    {"S_calcarine",12145},
    {"S_central",12146},
    {"S_cingul-Marginalis",12147},
    {"S_circular_insula_ant",12148},
    {"S_circular_insula_inf",12149},
    {"S_circular_insula_sup",12150},
    {"S_collat_transv_ant",12151},
    {"S_collat_transv_post",12152},
    {"S_front_inf",12153},
    {"S_front_middle",12154},
    {"S_front_sup",12155},
    {"S_interm_prim-Jensen",12156},
    {"S_intrapariet_andP_trans",12157},
    {"S_oc_middle_and_Lunatus",12158},
    {"S_os_sup_and_transversal",12159},
    {"S_occipital_ant",12160},
    {"S_oc-temp_lat",12161},
    {"S_oc-temp_med_and_Lingual",12162},
    {"S_orbital_lateral",12163},
    {"S_orbital_med-olfact",12164},
    {"S_orbital-H_Shaped",12165},
    {"S_parieto_occipital",12166},
    {"S_pericallosal",12167},
    {"S_postcentral",12168},
    {"S_precentral-inf-part",12169},
    {"S_precentral-sup-part",12170},
    {"S_suborbital",12171},
    {"S_subparietal",12172},
    {"S_temporal_inf",12173},
    {"S_temporal_sup",12174},
    {"S_temporal_transverse",12175}
};

QMap<QString, int> DestrieuxAtlasL = {
    {"Corpus",11100},
    {"G_and_S_frontomargin",11101},
    {"G_and_S_occipital_inf",11102},
    {"G_and_S_paracentral",11103},
    {"G_and_S_subcentral",11104},
    {"G_and_S_transv_frontopol",11105},
    {"G_and_S_cingul-Ant",11106},
    {"G_and_S_cingul-Mid-Ant",11107},
    {"G_and_S_cingul-Mid-Post",11108},
    {"G_cingul-Post-dorsal",11109},
        {"G_cuneus",11111},
    {"G_front_inf-Opercular",11112},
    {"G_front_inf-Orbital",11113},
    {"G_front_inf-Triangul",11114},
    {"G_front_middle",11115},
    {"G_front_sup",11116},
    {"G_Ins_lg_and_S_cent_ins",11117},
    {"G_insular_short",11118},
    {"G_and_S_cingul-Mid-Post",11108},
    {"G_occipital_middle",11119},
    {"G_occipital_sup",11120},
    {"G_oc-temp_med-Lingual",11122},
    {"G_oc-temp_med-Parahip",11123},
    {"G_orbital",11124},
    {"G_pariet_inf-Angular",11125},
    {"G_pariet_inf-Supramar",11126},
    {"G_parietal_sup",11127},
    {"G_postcentral",11128},
    {"G_precentral",11129},
    {"G_precunneus",11130},
    {"G_rectus",11131},
    {"G_subcallosal",11132},
    {"G_temp_sup-G_T_transv",11133},
    {"G_temp_sup-Lateral",11134},
    {"G_temp_sup-Plan_polar",11135},
    {"G_temp_sup-Plan_tempo",11136},
    {"G_temporal_inf",11137},
    {"G_temporal_middle",11138},
    {"Lat_Fis-ant-Horizont",11139},
    {"Lat_Fis-ant-Vertical",11140},
    {"Lat_Fis-post",11141},
    {"Pole_occipital",11143},
    {"Pole_temporal",11144},
    {"S_calcarine",11145},
    {"S_central",11146},
    {"S_cingul-Marginalis",11147},
    {"S_circular_insula_ant",11148},
    {"S_circular_insula_inf",11149},
    {"S_circular_insula_sup",11150},
    {"S_collat_transv_ant",11151},
    {"S_collat_transv_post",11152},
    {"S_front_inf",11153},
    {"S_front_middle",11154},
    {"S_front_sup",11155},
    {"S_interm_prim-Jensen",11156},
    {"S_intrapariet_andP_trans",11157},
    {"S_oc_middle_and_Lunatus",11158},
    {"S_os_sup_and_transversal",11159},
    {"S_occipital_ant",11160},
    {"S_oc-temp_lat",11161},
    {"S_oc-temp_med_and_Lingual",11162},
    {"S_orbital_lateral",11163},
    {"S_orbital_med-olfact",11164},
    {"S_orbital-H_Shaped",11165},
    {"S_parieto_occipital",11166},
    {"S_pericallosal",11167},
    {"S_postcentral",11168},
    {"S_precentral-inf-part",11169},
    {"S_precentral-sup-part",11170},
    {"S_suborbital",11171},
    {"S_subparietal",11172},
    {"S_temporal_inf",11173},
    {"S_temporal_sup",11174},
    {"S_temporal_transverse",11175}
};

vtkActor* selectedActor = nullptr;

CBrainAreaWidget::CBrainAreaWidget(QWidget* parent)
    :QWidget(parent)
    , ui(new Ui::CBrainAreaWidget())
    , vtkWidget(new QVTKOpenGLNativeWidget(this))
    , initTimer(nullptr)
{
    LOG_INFO("CBrainAreaWidget...................start");
    QElapsedTimer timer;
    timer.start();
    ui->setupUi(this);
    ui->dispProp->hide();
    ui->vtkContainer->addWidget(vtkWidget);
    vtkWidget->hide();
    movie = new QMovie(":/Optimus/images/common/round-progress.gif");
    movie->setScaledSize(QSize(ui->loadingLabel->width(), ui->loadingLabel->width()));
    ui->loadingLabel->setMovie(movie);
    movie->start();

    initAtlasTree();
    initVtk();
    LOG_INFO("CBrainAreaWidget...................finish");
}

CBrainAreaWidget::~CBrainAreaWidget() = default;

void CBrainAreaWidget::slotRoughnessChanged(int val)
{
    const auto roughness = static_cast<double>(val) / 100.0;
    brainUtility->setRoughness(roughness);
}

void CBrainAreaWidget::slotMetallicChanged(int val)
{
    auto metallic = static_cast<double>(val) / 100.0;
    brainUtility->setMetallic(metallic);
}

void CBrainAreaWidget::slotExposureChanged(int val)
{
    auto exposure = static_cast<double>(val) * 5.0 / 100.0;
    brainUtility->setExposure(exposure);
}


void CBrainAreaWidget::slotAreaChanged(QStandardItem* item)
{
    if(item->hasChildren()) // 如果有子节点选中状态与父节点不一致，使子节点与父节点选中状态相同
    {
        for(auto i = 0; i < item->rowCount(); ++i)
        {
            const auto child = item->child(i, 0);
            if(child->checkState() != item->checkState())
            {
                child->setCheckState(item->checkState());
            }
        }
    }
    else // TODO 在勾选所有选项结束后再去渲染，性能会好很多
    {
        static QSet<QString> visibleIds = treeItemLabelMap.keys().toSet();
        const auto selectedId = QString::number(item->parent()->parent()->row()) + "_" + QString::number(item->parent()->row()) + "_" + QString::number(item->row());

        const bool enabled = item->checkState() == Qt::Checked;
        if (enabled && !visibleIds.contains(selectedId))
        {
            visibleIds.insert(selectedId);
        }
        else if (!enabled && visibleIds.contains(selectedId))
        {
            visibleIds.remove(selectedId);
        }

        QVector<int> labels;
        for (auto id : visibleIds)
        {
            labels.append(treeItemLabelMap[id]);
        }

        brainUtility->showBrainArea(labels);
    }
}

void CBrainAreaWidget::slotOpacityChanged(int index)
{
    brainUtility->setOpacity(static_cast<double>(index) / 100.0);
}

void CBrainAreaWidget::slotUpdateDetectorTransform()
{
    const auto x = ui->xSlider->value();
    const auto y = ui->ySlider->value();
    const auto z = ui->zSlider->value();
    const auto a = ui->aSlider->value();
    const auto b = ui->bSlider->value();
    const auto c = ui->cSlider->value();
    brainUtility->updateDetectorTransform(x, y, z, a, b, c);

    const auto planIndex = ui->planIndexs->currentText();
    if(ui->actorType->currentIndex() == 0) // 电极
    {
        emit signalUpdateProbeTransform(planIndex, QVector3D(x, y, z), QVector3D(a, b, c));
    }
    else
    {
        emit signalUpdateDetectorTransform(planIndex, QVector3D(x, y, z), QVector3D(a, b, c));
    }

}

void CBrainAreaWidget::slotPlanIndexesChanged(QString text)
{
    brainUtility->setSelectedTag(text);
    double x, y, z, a, b, c;
    brainUtility->getSelectedActorTransform(x, y, z, a, b, c);
    ui->xSlider->setValue(x);
    ui->ySlider->setValue(y);
    ui->zSlider->setValue(z);
    ui->aSlider->setValue(a);
    ui->bSlider->setValue(b);
    ui->cSlider->setValue(c);
}

void CBrainAreaWidget::slotIsProbeChanged(int index)
{
    brainUtility->setIsSelectProbe(index == 0);
    if(ui->planIndexs->count() >0)
    {
        double x, y, z, a, b, c;
        brainUtility->getSelectedActorTransform(x, y, z, a, b, c);
        ui->xSlider->setValue(x);
        ui->ySlider->setValue(y);
        ui->zSlider->setValue(z);
        ui->aSlider->setValue(a);
        ui->bSlider->setValue(b);
        ui->cSlider->setValue(c);
    }
}

void CBrainAreaWidget::slotSetLeftHemisphereVisible(bool visible)
{
    if(brainUtility->getInitResult())
    {
        brainUtility->setLeftHemisphereVisible(visible);
    }
}

void CBrainAreaWidget::slotSetRightHemisphereVisible(bool visible)
{
    if (brainUtility->getInitResult())
    {
        brainUtility->setRightHemisphereVisible(visible);
    }
}

void CBrainAreaWidget::slotSetDetectorVisible(bool visible)
{
    if(brainUtility->getInitResult())
    {
        brainUtility->setDetectorVisible(visible);
    }
}

void CBrainAreaWidget::slotSetProbeVisible(bool visible)
{
    if (brainUtility->getInitResult())
    {
        brainUtility->setProbeVisible(visible);
    }
}

void CBrainAreaWidget::setAxesVisible(bool visible)
{
    if (brainUtility->getInitResult())
    {
        brainUtility->setAxesVisible(visible);
    }
}

void CBrainAreaWidget::initVtk()
{
    if(!brainUtility->getInitResult())
    {
        brainUtility->asyncInit();
    }
    brainUtility->clearDetectors();

    if(!initTimer)
    {
        initTimer = new QTimer(this);
        initTimer->setInterval(5);
        static QElapsedTimer refreshTimer;
        refreshTimer.start();

        connect(initTimer, &QTimer::timeout, [this]()
            {
                if(refreshTimer.elapsed() > 500)
                {
                    if (ui->loadingEllipsis->text().length() >= 6)
                    {
                        ui->loadingEllipsis->setText("");
                    }
                    else
                    {
                        ui->loadingEllipsis->setText(ui->loadingEllipsis->text() + " .");
                    }
                    refreshTimer.restart();
                }
                
                if (brainUtility->getInitResult())
                {
                    renderWindow = brainUtility->getRenderWindow();
                    initTimer->stop();
                    
                    setSignals();
                    emit signalVtkInited();
                }
            });
        initTimer->start();
    }
}

void CBrainAreaWidget::initAtlasTree()
{
    const auto atlas = brainAtlasCfg.getAtlas();
    atlasModel = new QStandardItemModel();
    ui->atlas->setModel(atlasModel);

    std::string hemisphereNames[] = { "Left Hemisphere" ,  "Right Hemisphere" };
    if(systemConfig.GetSystemCfg().Common.Language == "zh-CN")
    {
        hemisphereNames[0] = QString::fromLocal8Bit("左脑").toStdString();
        hemisphereNames[1] = QString::fromLocal8Bit("右脑").toStdString();
    }

    const std::vector<BrainLobeInfo> hemispheres[] = { atlas.LeftHemisphere, atlas.RightHemisphere };

    for (int i : {0, 1})
    {
        const auto leftHemisphere = new QStandardItem(tr(hemisphereNames[i].c_str()));
        leftHemisphere->setCheckable(true);
        leftHemisphere->setCheckState(Qt::Checked);
        for (auto lobe : hemispheres[i])
        {
            const auto brainLobeInfo = new QStandardItem(QString::fromStdString(lobe.ChineseName));
            leftHemisphere->appendRow(brainLobeInfo);
            brainLobeInfo->setCheckable(true);
            brainLobeInfo->setCheckState(Qt::Checked);
            for (auto area : lobe.Areas)
            {
                const auto areaInfo = new QStandardItem(QString::fromStdString(area.ChineseName));
                areaInfo->setCheckable(true);
                areaInfo->setCheckState(Qt::Checked);
                brainLobeInfo->appendRow(areaInfo);

                auto id = QString::number(i) + "_" + QString::number(brainLobeInfo->row()) + "_"+ QString::number(areaInfo->row());
                treeItemLabelMap.insert(id, area.Label);

            }
            
        }
        atlasModel->appendRow(leftHemisphere);
    }
}

void CBrainAreaWidget::setSignals() const
{
    const vtkNew<QVTKInteractor> interactor;

    vtkWidget->setRenderWindow(renderWindow); // set RenderWindow时会自动创建QVtkInteractor, Style为 TrackBall

    ui->loadingContainer->hide();
    vtkWidget->show();

    brainUtility->setRoughness(static_cast<double>(ui->roughnessSlider->value()) / 100.0);
    brainUtility->setExposure(static_cast<double>(ui->exposureSlider->value()) * 5.0 / 100.0);
    brainUtility->setMetallic(static_cast<double>(ui->metallicitySlider->value()) / 100.0);

    connect(ui->roughnessSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRoughnessChanged(int)));
    connect(ui->metallicitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotMetallicChanged(int)));
    connect(ui->exposureSlider, SIGNAL(valueChanged(int)), this, SLOT(slotExposureChanged(int)));
    connect(atlasModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(slotAreaChanged(QStandardItem*)));
    connect(ui->xSlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->ySlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->zSlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->aSlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->bSlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->cSlider, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateDetectorTransform()));
    connect(ui->planIndexs, SIGNAL(currentTextChanged(QString)), this, SLOT(slotPlanIndexesChanged(QString)));
    connect(ui->actorType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIsProbeChanged(int)));

    connect(ui->leftHemisphere, SIGNAL(clicked(bool)), this, SLOT(slotSetLeftHemisphereVisible(bool)));
    connect(ui->rightHemisphere, SIGNAL(clicked(bool)), this, SLOT(slotSetRightHemisphereVisible(bool)));
    connect(ui->showDetector, SIGNAL(clicked(bool)), this, SLOT(slotSetDetectorVisible(bool)));
    connect(ui->showProbe, SIGNAL(clicked(bool)), this, SLOT(slotSetProbeVisible(bool)));
    connect(ui->showAxis, SIGNAL(clicked(bool)), this, SLOT(setAxesVisible(bool)));
}

bool CBrainAreaWidget::addDetectorAndProbes(const QString& tag, const BrainAreaInfo& detectorPos, const BrainAreaInfo& probePos, const QString& detectorType_) const
{
    if(detectorType_ != "G3" && detectorType_ != "G2+") // TODO 当前仅有 G3 G2+的配置
    {
        return false;
    }

    if(brainUtility->getInitResult())
    {
        const auto res = brainUtility->addDetectorAndProbe(tag, detectorPos, probePos, detectorType_);
        if (res)
        {
            ui->planIndexs->addItem(tag);
            ui->planIndexs->setCurrentText(tag);
            double x, y, z, a, b, c;
            brainUtility->getSelectedActorTransform(x, y, z, a, b, c);
            ui->xSlider->setValue(x);
            ui->ySlider->setValue(y);
            ui->zSlider->setValue(z);
            ui->aSlider->setValue(a);
            ui->bSlider->setValue(b);
            ui->cSlider->setValue(c);
        }
        return res;
    }

    return false;
}

void CBrainAreaWidget::removeDetectorAndProbe(const QString& tag)
{
    if(brainUtility->getInitResult())
    {
        for (auto i = 0; i < ui->planIndexs->count(); ++i)
        {
            if (ui->planIndexs->itemText(i) == tag)
            {
                ui->planIndexs->removeItem(i);
                break;
            }
        }
        brainUtility->removeDetectorAndProbe(tag);
    }
}

void CBrainAreaWidget::initBrainModel()
{
    brainUtility->asyncInit();
}


END_NX_NAMESPACE


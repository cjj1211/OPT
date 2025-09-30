#include "CBrainAreaUtility.h"

#include <QVector3D>

#include "ColorMap.h"
#include <ylt/struct_json/json_reader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkLight.h>
#include <vtkCameraPass.h>
#include <vtkSequencePass.h>
#include <vtkOpaquePass.h>
#include <vtkLightsPass.h>
#include <vtkNamedColors.h>
#include <vtkOverlayPass.h>
#include <vtkRenderPassCollection.h>
#include <vtkToneMappingPass.h>
#include <vtkPBRIrradianceTexture.h>
#include <vtkTransform.h>
#include <vtkMath.h>
#include <vtkAxesActor.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "AtlasConfiguration.h"
#include "DetectorConfiguration.h"

BEGIN_NX_NAMESPACE
    CBrainAreaUtility::CBrainAreaUtility() = default;

CBrainAreaUtility* CBrainAreaUtility::getInstance()
{
    static CBrainAreaUtility* instance = new CBrainAreaUtility;
    return instance;
}

void CBrainAreaUtility::asyncInit()
{
    static bool isInInit = false; // 正在初始化过程中。。。

    if(!isInInit && !isInited)
    {
        isInInit = true;
        QtConcurrent::run([this]()
            {
                auto filepath = R"(..\config\BrainArea\freesurfer.json)";
                ColorMap freeSufferMap;
                if (!QFile::exists(filepath))
                {
                    LOG_ERR(std::format("File not exist: {}", filepath));
                    return;
                }
                struct_json::from_json_file(freeSufferMap, filepath);

                for (const auto& [label, color] : freeSufferMap.relations)
                {
                    colorMap.insert(label, color.vect);
                }

                createRenderWindow();
                isInInit = false;
            });
    }
}

bool CBrainAreaUtility::getInitResult() const
{
    return isInited;
}

CBrainAreaUtility::~CBrainAreaUtility() = default;

void CBrainAreaUtility::createRenderer()
{
    // TODO 需要通过参数传入
    // Desikan / Killiany Atlas 分割出35个脑区 保存在 * .aparc.annot
    // Destrieux Atlas 分割出75个脑区，保存在 * .aparc.a2009s.annot中
    auto loadLeftHeadActor = QtConcurrent::run([&]() {
        const auto lhModelPath = R"(..\config\BrainArea\lh.vtk)";
        const auto lhLutPath = R"(..\config\BrainArea\lh.native.label.aparc.txt\lh.native.label.aparc.txt)";
        lhActor = createActor(lhModelPath, lhLutPath, leftLabelVec);
        });

    auto loadRightHeadActor = QtConcurrent::run([&]() {
        const auto rhModelPath = R"(..\config\BrainArea\rh.vtk)";
        const auto rhLutPath = R"(..\config\BrainArea\rh.native.label.aparc.txt\rh.native.label.aparc.txt)";
        rhActor = createActor(rhModelPath, rhLutPath, rightLabelVec);
        });

    loadLeftHeadActor.waitForFinished();
    loadRightHeadActor.waitForFinished();
    
    const double bg[] = { 22 / 255.0, 36 / 255.0, 85 / 255.0 };
    auto colors = vtkNew<vtkNamedColors>();
    renderer = vtkNew<vtkOpenGLRenderer>();
    renderer->SetBackground(bg);
    renderer->AddActor(lhActor);
    renderer->AddActor(rhActor);
    renderer->AddActor(worldAxes);

    // set lights 
    const auto light = vtkNew<vtkLight>();
    const auto light2 = vtkNew<vtkLight>();
    const auto light3 = vtkNew<vtkLight>();
    const auto light4 = vtkNew<vtkLight>();

    light->SetFocalPoint(0, 0, 0);
    light2->SetFocalPoint(0, 0, 0);
    light3->SetFocalPoint(0, 0, 0);
    light4->SetFocalPoint(0, 0, 0);

    light->SetPosition(-20.8, -10.6, 1);
    light2->SetPosition(20.8, -10.6, 1);
    light3->SetPosition(0, 10.6, 1);
    light4->SetPosition(2, 3, -10);

    renderer->AddLight(light);
    renderer->AddLight(light2);
    renderer->AddLight(light3);
    renderer->AddLight(light4);

    // set multi pass
    const auto cameraPass = vtkNew<vtkCameraPass>();
    const auto seqPass = vtkNew<vtkSequencePass>();
    const auto opaque = vtkNew<vtkOpaquePass>();
    const auto lights = vtkNew<vtkLightsPass>();
    const auto overlay = vtkNew<vtkOverlayPass>();

    const auto passes = vtkNew<vtkRenderPassCollection>();
    passes->AddItem(lights);
    passes->AddItem(opaque);
    passes->AddItem(overlay);
    seqPass->SetPasses(passes);
    cameraPass->SetDelegatePass(seqPass);
    toneMapping = vtkNew<vtkToneMappingPass>();
    toneMapping->SetDelegatePass(cameraPass);
    toneMapping->SetExposure(exposure);
    renderer->SetPass(toneMapping);

    vtkOpenGLRenderer::SafeDownCast(renderer)->SetPass(toneMapping);
    const vtkSmartPointer<vtkPBRIrradianceTexture> irradiance = renderer->GetEnvMapIrradiance();
    irradiance->SetIrradianceStep(0.3);
}

void CBrainAreaUtility::createRenderWindow()
{
    QElapsedTimer timer;
    timer.start();
    createRenderer();
    timer.restart();
    window->AddRenderer(renderer);
    isInited = true;
}

vtkGenericOpenGLRenderWindow* CBrainAreaUtility::getRenderWindow() const
{
    if(isInited)
    {
        return window.Get();
    }
    return nullptr;
}

void CBrainAreaUtility::setRoughness(const double value) const
{
    lhActor->GetProperty()->SetRoughness(value);
    rhActor->GetProperty()->SetRoughness(value);
    window->Render();
    
}

void CBrainAreaUtility::setMetallic(const double value) const
{
    lhActor->GetProperty()->SetMetallic(value);
    rhActor->GetProperty()->SetMetallic(value);
    window->Render();
}

void CBrainAreaUtility::setExposure(const double value) const
{
    toneMapping->SetExposure(value);
    window->Render();
}

void CBrainAreaUtility::setOpacity(const double value)
{
    opacity = value;
}

void CBrainAreaUtility::setLeftHemisphereVisible(bool visible) const
{
    lhActor->SetVisibility(visible);
    window->Render();
}

bool CBrainAreaUtility::getLeftHemisphereVisible() const
{
    return lhActor->GetVisibility();
}

void CBrainAreaUtility::setRightHemisphereVisible(bool visible) const
{
    rhActor->SetVisibility(visible);
    window->Render();
}

bool CBrainAreaUtility::getRightHemisphereVisible() const
{
    return rhActor->GetVisibility();
}

void CBrainAreaUtility::setDetectorVisible(const bool visible)
{
    for(const auto assembly : detectorAssemblies)
    {
        assembly.actor->SetVisibility(visible);
    }
    detectorVisible = visible;
    window->Render();
}

void CBrainAreaUtility::setProbeVisible(const bool visible)
{
    for (const auto assembly : probeAssemblies)
    {
        assembly.actor->SetVisibility(visible);
    }
    probeVisible = visible;
    window->Render();
}

void CBrainAreaUtility::setAxesVisible(const bool visible)
{
    for (const auto assembly : probeAssemblies)
    {
        assembly.axesActor->SetVisibility(visible);
    }

    for (const auto assembly : detectorAssemblies)
    {
        assembly.axesActor->SetVisibility(visible);
    }

    worldAxes->SetVisibility(visible);
    axesVisible = visible;
    window->Render();
}


void CBrainAreaUtility::showBrainArea(const QVector<int>  labels)
{
    auto func = [=](vtkMapper* mapper, const QVector<int>& labelVec) {
        const auto tableSize = mapper->GetLookupTable()->GetNumberOfAvailableColors();

        const auto lut = vtkNew<vtkLookupTable>();
        lut->SetNumberOfTableValues(tableSize);
        lut->SetScaleToLinear();
        lut->Build();

        for (int i = 0; i < tableSize; i++)
        {
            vtkColor4d color;
            int label = labelVec[i];

            if(label == 0 || label == 660700)
            {
                color.SetRed(0);
                color.SetBlue(0);
                color.SetGreen(0);
                color.SetAlpha(opacity);
            }
            else if (labels.contains(labelVec[i]) || labels.empty())
            {
                color.SetRed(colorMap[label][0]);
                color.SetBlue(colorMap[label][1]);
                color.SetGreen(colorMap[label][2]);
                color.SetAlpha(opacity);
            }
            else
            {
                color.SetRed(1);
                color.SetBlue(1);
                color.SetGreen(1);
                color.SetAlpha(opacity);
            }
            lut->SetTableValue(i, color.GetData());
        }
        mapper->SetLookupTable(lut);
    };

    func(lhActor->GetMapper(), leftLabelVec);
    func(rhActor->GetMapper(), rightLabelVec);
    window->Render();
}

bool CBrainAreaUtility::addDetectorAndProbe(const QString& tag, const BrainAreaInfo& detectorPos, const BrainAreaInfo& probePos, const QString& detectorType_)
{
    if (!isInited) return false;
    auto res = true;
    res = res && addDetector(tag, detectorPos, detectorType_);
    res = res && addProbe(tag, probePos, detectorType_);
    if(res)
    {
        selectedTag = tag;
    }
    return res;
}

void CBrainAreaUtility::removeDetectorAndProbe(const QString& tag)
{
    if(detectorAssemblies.contains(tag))
    {
        renderer->RemoveActor(detectorAssemblies[tag].actor);
        renderer->RemoveActor(detectorAssemblies[tag].axesActor);
        detectorAssemblies.remove(tag);
    }

    if(probeAssemblies.contains(tag))
    {
        renderer->RemoveActor(probeAssemblies[tag].actor);
        renderer->RemoveActor(probeAssemblies[tag].axesActor);
        probeAssemblies.remove(tag);
    }
    window->Render();
}

void CBrainAreaUtility::clearDetectors()
{
    for(const auto detectorModel : detectorAssemblies)
    {
        renderer->RemoveActor(detectorModel.actor);
        renderer->RemoveActor(detectorModel.axesActor);
    }
    detectorAssemblies.clear();

    for(const auto probeModel : probeAssemblies)
    {
        renderer->RemoveActor(probeModel.actor);
        renderer->RemoveActor(probeModel.axesActor);
    }
    probeAssemblies.clear();
}

void CBrainAreaUtility::updateDetectorTransform(const double x, const double y, const double z, const double a, const double b, const double c)
{
    AssemblyModel* selectedModel;
    if(isSelectProbe)
    {
        selectedModel = &probeAssemblies[selectedTag];
    }
    else
    {
        selectedModel = &detectorAssemblies[selectedTag];
    }
    selectedModel->translate.setX(x);    // NOLINT(bugprone-narrowing-conversions)
    selectedModel->translate.setY(y);    // NOLINT(bugprone-narrowing-conversions)
    selectedModel->translate.setZ(z);    // NOLINT(bugprone-narrowing-conversions)
    selectedModel->rotate.setX(a);       // NOLINT(bugprone-narrowing-conversions)
    selectedModel->rotate.setY(b);       // NOLINT(bugprone-narrowing-conversions)
    selectedModel->rotate.setZ(c);       // NOLINT(bugprone-narrowing-conversions)

    const auto trans = vtkNew<vtkTransform>();
    trans->PostMultiply();
    // 移动到模型坐标原点
    trans->Translate(-selectedModel->origenPos.x(), -selectedModel->origenPos.y(), -selectedModel->origenPos.z());

    // 将模型法线旋转到与世界坐标y轴同轴
    constexpr double targetNormal[] = { 0, 0, 1 };
    const double currentNormal[] = { selectedModel->normal.x(),  selectedModel->normal.y(),  selectedModel->normal.z() };
    auto angle = vtkMath::AngleBetweenVectors(currentNormal, targetNormal);
    angle = vtkMath::DegreesFromRadians(angle);
    double axis[3];
    vtkMath::Cross(currentNormal, targetNormal, axis);
    trans->RotateWXYZ(angle, axis);

    // 旋转到脑区方向
    trans->RotateX(selectedModel->rotate.x());
    trans->RotateY(selectedModel->rotate.y());
    trans->RotateZ(selectedModel->rotate.z());

    // 移动到脑区
    trans->Translate(selectedModel->translate.x(), selectedModel->translate.y(), selectedModel->translate.z()); // 移动到模型坐标原点
    
    selectedModel->actor->SetUserTransform(trans);

    const auto trans2 = vtkNew<vtkTransform>();
    trans2->PostMultiply();
    trans2->RotateX(selectedModel->rotate.x());
    trans2->RotateY(selectedModel->rotate.y());
    trans2->RotateZ(selectedModel->rotate.z());
    trans2->Translate(selectedModel->translate.x(), selectedModel->translate.y(), selectedModel->translate.z()); // 移动到模型坐标原点

    selectedModel->axesActor->SetUserTransform(trans2);

    window->Render();
}

vtkNew<vtkActor> CBrainAreaUtility::createActor(const std::string& modelPath, const std::string& lutPath, QVector<int>& labelVec)
{
    QElapsedTimer timer;
    timer.restart();

    const auto reader = vtkNew<vtkPolyDataReader>();
    reader->SetFileName(modelPath.c_str());
    reader->Update();
    //qDebug() << "Read file: " << timer.elapsed();
    timer.restart();
    // Set lut
    const auto polyData = reader->GetOutput();
    const auto tableSize = polyData->GetNumberOfPoints();

    auto a = polyData->GetPointData();

    const auto scalars = vtkNew<vtkFloatArray>();
    scalars->SetName("scalars");
    for (int i = 0; i < tableSize; ++i)
    {
        scalars->InsertNextValue(i);
    }
    polyData->GetPointData()->SetScalars(scalars);
    qDebug() << "Set scalar:" << timer.elapsed();
    timer.restart();
    // smooth mesh
    const auto smoothFilter = vtkNew<vtkSmoothPolyDataFilter>();
    smoothFilter->SetInputConnection(reader->GetOutputPort());
    smoothFilter->SetNumberOfIterations(50); // 迭代次数
    smoothFilter->SetRelaxationFactor(0.1); // 平滑因子
    smoothFilter->Update();
    qDebug() << "Smooth mesh:" << timer.elapsed();
    timer.restart();

    // 更新法线
    const auto normals = vtkNew<vtkPolyDataNormals>();
    normals->SetInputConnection(smoothFilter->GetOutputPort());
    normals->ComputeCellNormalsOn();
    normals->ComputePointNormalsOn();
    normals->Update();
    qDebug() << "Update normals:" << timer.elapsed();
    timer.restart();

    // SET LUT
    const auto lut = readLUT(tableSize, lutPath, labelVec);

    // Create mapper
    const auto mapper = vtkNew<vtkPolyDataMapper>();
    mapper->SetInputConnection(normals->GetOutputPort());
    mapper->SetLookupTable(lut);
    mapper->SetScalarRange(0, tableSize);
    mapper->Update();
    qDebug() << "Set mapper:" << timer.elapsed();

    const auto colors = vtkNew<vtkNamedColors>();
    auto actor = vtkNew<vtkActor>();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetInterpolationToPBR(); // 设置插值
    actor->GetProperty()->SetColor(colors->GetColor3d("White").GetData());
    actor->GetProperty()->SetRoughness(roughness);
    actor->GetProperty()->SetMetallic(metallic);
    worldAxes = vtkNew<vtkAxesActor>();
    worldAxes->SetTotalLength(100, 100, 100);
    worldAxes->SetVisibility(axesVisible);
    qDebug() <<"createActor:" << timer.elapsed();
    return actor;
}

vtkNew<vtkLookupTable> CBrainAreaUtility::readLUT(int tableSize, const std::string& lutPath,  QVector<int>& labelVec)
{
    std::lock_guard lock(lutMutex);
    auto lut = vtkNew<vtkLookupTable>();
    lut->SetNumberOfTableValues(tableSize);
    lut->SetScaleToLinear();
    lut->Build();

    // 从freesurfer.json中获取颜色表
    std::ifstream fin(lutPath);
    std::string line;
    int lineNum = 0;
    
    while (std::getline(fin, line))
    {
        auto label = std::atoi(line.c_str());
        if(label == 1)
        {
            qDebug() << QString::fromStdString(line);
        }

        if (lineNum < tableSize)
        {
            vtkColor4d color;
            if(label==0 || label == 660700)
            {
                color.SetRed(0);
                color.SetBlue(0);
                color.SetGreen(0);
            }
            else
            {
                color.SetRed(colorMap[label][0]);
                color.SetBlue(colorMap[label][1]);
                color.SetGreen(colorMap[label][2]);
            }
            
            color.SetAlpha(opacity);
            lut->SetTableValue(lineNum, color.GetData());
            labelVec.append(label);
            lutLabelSet.insert(label);
        }
        else
        {
            break;
        }

        lineNum++;
    }
    fin.close();

    return lut;
}

bool CBrainAreaUtility::addProbe(const QString& tag, const BrainAreaInfo& areaInfo, const QString& detectorType_)
{
    if (!isInited) return false;
    std::string type = detectorType_.toStdString();
    auto detectorCfg = CDetectorConfig::getConfig(std::format(R"(..\config\DetectorModel\{}\{}.json)", type, type));

    AssemblyModel probeModel;
    probeModel.origenPos = {
        static_cast<float>(detectorCfg.ProbeCenter[0]),
        static_cast<float>(detectorCfg.ProbeCenter[1]),
        static_cast<float>(detectorCfg.ProbeCenter[2]) };
    probeModel.normal = {
        static_cast<float>(detectorCfg.ProbeNormal[0]),
        static_cast<float>(detectorCfg.ProbeNormal[1]),
        static_cast<float>(detectorCfg.ProbeNormal[2]) };
    probeModel.translate.setX(areaInfo.Center[0]);
    probeModel.translate.setY(areaInfo.Center[1]);
    probeModel.translate.setZ(areaInfo.Center[2]);
    probeModel.rotate.setX(areaInfo.Rotate[0]);
    probeModel.rotate.setY(areaInfo.Rotate[1]);
    probeModel.rotate.setZ(areaInfo.Rotate[2]);

    const auto reader = vtkNew<vtkSTLReader>();
    reader->SetFileName(std::format(R"(..\config\DetectorModel\{}\{}_Probe.stl)", type, type).c_str());
    reader->Update();

    // Create mapper
    const auto mapper = vtkNew<vtkPolyDataMapper>();
    mapper->SetInputConnection(reader->GetOutputPort());
    mapper->Update();

    const auto colors = vtkNew<vtkNamedColors>();
    const vtkNew<vtkActor> probeActor;
    probeActor->SetVisibility(probeVisible);
    probeModel.actor = probeActor;
    probeActor->SetMapper(mapper);
    probeActor->GetProperty()->SetInterpolationToPBR(); // PBR渲染
    probeActor->GetProperty()->SetColor(colors->GetColor3d("Gold").GetData());
    probeActor->GetProperty()->SetRoughness(roughness);
    probeActor->GetProperty()->SetMetallic(metallic);

    const auto trans = vtkNew<vtkTransform>();
    trans->PostMultiply();
    trans->Translate(-detectorCfg.ProbeCenter[0], -detectorCfg.ProbeCenter[1], -detectorCfg.ProbeCenter[2]); // 移动到模型坐标原点

    // 使电极法向量与世界坐标系法向量重合
    constexpr double targetNormal[] = { 0, 0, 1 }; 
    auto angle = vtkMath::AngleBetweenVectors(detectorCfg.ProbeNormal.data(), targetNormal);
    angle = vtkMath::DegreesFromRadians(angle);
    double axis[3];
    vtkMath::Cross(detectorCfg.ProbeNormal.data(), targetNormal, axis);
    trans->RotateWXYZ(angle, axis);

    // 旋转到指定脑区的角度
    trans->RotateX(probeModel.rotate.x());
    trans->RotateY(probeModel.rotate.y());
    trans->RotateZ(probeModel.rotate.z());

    // 移动到指定脑区位置
    trans->Translate(probeModel.translate.x(), probeModel.translate.y(), probeModel.translate.z()); // 移动到指定脑区中心

    probeActor->SetUserTransform(trans);
    renderer->AddActor(probeActor);


    const auto axisActor = vtkNew<vtkAxesActor>();
    axisActor->SetVisibility(axesVisible);
    probeModel.axesActor = axisActor;
    const auto trans2 = vtkNew<vtkTransform>();
    trans2->PostMultiply();
    trans2->RotateX(probeModel.rotate.x());
    trans2->RotateY(probeModel.rotate.y());
    trans2->RotateZ(probeModel.rotate.z());
    trans2->Translate(probeModel.translate.x(), probeModel.translate.y(), probeModel.translate.z());// 移动到模型坐标原点
    axisActor->SetUserTransform(trans2);
    axisActor->SetTotalLength(30, 30, 30);
    renderer->AddActor(axisActor);

    window->Render();

    probeAssemblies.insert(tag, probeModel);
    return true;
}

bool CBrainAreaUtility::addDetector(const QString& tag, const BrainAreaInfo& areaInfo, const QString& detectorType_)
{
    std::string type = detectorType_.toStdString();
    auto detectorCfg = CDetectorConfig::getConfig(std::format(R"(..\config\DetectorModel\{}\{}.json)", type, type));

    AssemblyModel detectorModel;
    detectorModel.origenPos ={
        static_cast<float>(detectorCfg.DetectorCenter[0]),
        static_cast<float>(detectorCfg.DetectorCenter[1]),
        static_cast<float>(detectorCfg.DetectorCenter[2]) };
    detectorModel.normal = {
        static_cast<float>(detectorCfg.DetectorNormal[0]),
        static_cast<float>(detectorCfg.DetectorNormal[1]),
        static_cast<float>(detectorCfg.DetectorNormal[2]) };
    detectorModel.translate.setX(areaInfo.Center[0]);
    detectorModel.translate.setY(areaInfo.Center[1]);
    detectorModel.translate.setZ(areaInfo.Center[2]);
    detectorModel.rotate.setX(areaInfo.Rotate[0]);
    detectorModel.rotate.setY(areaInfo.Rotate[1]);
    detectorModel.rotate.setZ(areaInfo.Rotate[2]); 

    const auto reader = vtkNew<vtkSTLReader>();
    reader->SetFileName(std::format(R"(..\config\DetectorModel\{}\{}_Detector.stl)", type, type).c_str());
    reader->Update();

    // Create mapper
    const auto mapper = vtkNew<vtkPolyDataMapper>();
    mapper->SetInputConnection(reader->GetOutputPort());
    mapper->Update();

    const auto colors = vtkNew<vtkNamedColors>();
    const auto detectorActor = vtkNew<vtkActor>();
    detectorActor->SetVisibility(detectorVisible);
    detectorModel.actor = detectorActor;
    detectorActor->SetMapper(mapper);
    detectorActor->GetProperty()->SetInterpolationToPBR(); // PBR渲染
    detectorActor->GetProperty()->SetColor(colors->GetColor3d("White").GetData());
    detectorActor->GetProperty()->SetRoughness(roughness);
    detectorActor->GetProperty()->SetMetallic(metallic);

    const auto trans = vtkNew<vtkTransform>();
    trans->PostMultiply();
    // 移动到模型坐标原点
    trans->Translate(-detectorModel.origenPos.x(), -detectorModel.origenPos.y(), -detectorModel.origenPos.z());

    // 使探头法线方向与世界坐标z轴重合
    constexpr double targetNormal[] = { 0,0,1 };
    auto angle = vtkMath::AngleBetweenVectors(detectorCfg.DetectorNormal.data(), targetNormal);
    angle = vtkMath::DegreesFromRadians(angle);
    double axis[3];
    vtkMath::Cross(detectorCfg.DetectorNormal.data(), targetNormal, axis);
    trans->RotateWXYZ(angle, axis);

    // 旋转到指定脑区的角度
    trans->RotateX(detectorModel.rotate.x());
    trans->RotateY(detectorModel.rotate.y());
    trans->RotateZ(detectorModel.rotate.z());

    // 平移到指定脑区
    trans->Translate(detectorModel.translate.x(), detectorModel.translate.y(), detectorModel.translate.z()); // 移动到指定脑区中心

    detectorActor->SetUserTransform(trans);
    renderer->AddActor(detectorActor);


    const auto axisActor = vtkNew<vtkAxesActor>();
    axisActor->SetVisibility(axesVisible);
    const auto trans2 = vtkNew<vtkTransform>();
    trans2->PostMultiply();
    trans2->RotateX(detectorModel.rotate.x());
    trans2->RotateY(detectorModel.rotate.y());
    trans2->RotateZ(detectorModel.rotate.z());
    trans2->Translate(detectorModel.translate.x(), detectorModel.translate.y(), detectorModel.translate.z());// 移动到模型坐标原点
    axisActor->SetUserTransform(trans2);
    axisActor->SetTotalLength(30, 30, 30);
    detectorModel.axesActor = axisActor;
    renderer->AddActor(axisActor);

    window->Render();

    detectorAssemblies.insert(tag, detectorModel);
    return true;
}

void CBrainAreaUtility::setSelectedTag(const QString& tag)
{
    if(detectorAssemblies.keys().contains(tag))
    {
        selectedTag = tag;
    }
    else
    {
        LOG_ERR(std::format("Can not find tag {}", tag.toStdString()).c_str());
    }
}

void CBrainAreaUtility::setIsSelectProbe(const bool isSelect)
{
    isSelectProbe = isSelect;
}

void CBrainAreaUtility::getSelectedActorTransform(double& x, double& y, double& z, double& a, double& b, double& c)
{
    AssemblyModel selectedModel;
    if (isSelectProbe)
    {
        selectedModel = probeAssemblies[selectedTag];
    }
    else
    {
        selectedModel = detectorAssemblies[selectedTag];
    }
    
    x = selectedModel.translate.x();
    y = selectedModel.translate.y();
    z = selectedModel.translate.z();
    a = selectedModel.rotate.x();
    b = selectedModel.rotate.y();
    c = selectedModel.rotate.z();
}

END_NX_NAMESPACE


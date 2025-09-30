/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_BRAIN_AREA_UTILITY_H
#define __C_BRAIN_AREA_UTILITY_H
#include "nxbrainareawidget_global.h"
#include <QObject>
#include <QSet>
#include <QMap>
#include <QVector3D>
#include <QVector>
#include <mutex>

#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>
#include <vtkOpenGLRenderer.h>
#include <vtkToneMappingPass.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkPropPicker.h>

class vtkAxesActor;

namespace NX
{
    struct BrainAreaInfo;
}

BEGIN_NX_NAMESPACE

class CBrainAreaUtility
{
    struct AssemblyModel
    {
        vtkActor* actor;
        vtkAxesActor* axesActor;
        QVector3D origenPos;
        QVector3D normal;
        QVector3D translate;
        QVector3D rotate;
    };

public:
    CBrainAreaUtility(CBrainAreaUtility&) = delete;
    CBrainAreaUtility(CBrainAreaUtility&&) = delete;
    CBrainAreaUtility& operator=(CBrainAreaUtility&) = delete;
    CBrainAreaUtility&& operator=(CBrainAreaUtility&&) = delete;
    ~CBrainAreaUtility();

    static CBrainAreaUtility* getInstance();

    void asyncInit();
    bool getInitResult() const;
    vtkGenericOpenGLRenderWindow* getRenderWindow() const;
    void setRoughness(double value) const;
    void setMetallic(double value) const;
    void setExposure(double value) const;
    void setOpacity(double value);
    void setLeftHemisphereVisible(bool visible) const;
    bool getLeftHemisphereVisible() const;
    void setRightHemisphereVisible(bool visible) const;
    bool getRightHemisphereVisible() const;
    void setDetectorVisible(bool visible);
    void setProbeVisible(bool visible);
    void setAxesVisible(bool visible);

    void showBrainArea(QVector<int> labels);
    QSet<int> getLutSet() { return lutLabelSet; }

    bool addDetectorAndProbe(const QString& tag, const BrainAreaInfo& detectorPos, const BrainAreaInfo& probePos, const QString& detectorType_);
    void removeDetectorAndProbe(const QString& tag);
    void clearDetectors();

    void setSelectedTag(const QString& tag);
    void setIsSelectProbe(const bool isSelect);
    void getSelectedActorTransform(double& x, double& y, double& z, double& a, double& b, double& c);
    void updateDetectorTransform(double x, double y, double z, double a, double b, double c);
private:
    CBrainAreaUtility();
    
    void createRenderWindow();
    void createRenderer();
    vtkNew<vtkActor> createActor(const std::string& modelPath, const std::string& lutPath, QVector<int>& labelVec);
    vtkNew<vtkLookupTable> readLUT(int tableSize, const std::string& lutPath, QVector<int>& labelVec);
    bool addProbe(const QString& tag, const BrainAreaInfo& areaInfo, const QString& detectorType_);
    bool addDetector(const QString& tag, const BrainAreaInfo& areaInfo, const QString& detectorType_);

private:
    bool showDetector{ true };
    bool showProbe{ true };
    QString selectedTag{""}; // 选中的电极和探头
    bool isSelectProbe{ true }; // true 选中电极， false 选中 探头
    bool detectorVisible{ true };
    bool probeVisible{true};
    bool axesVisible{ false };


    double roughness{ 0.4 };
    double metallic{ 0.1 };
    double exposure{ 2.5 };
    double opacity{ 1.0 };
    QMap<int, std::vector<double>> colorMap;
    vtkNew<vtkActor> lhActor;
    vtkNew<vtkActor> rhActor;
    vtkNew<vtkAxesActor> worldAxes;
    QMap<QString, AssemblyModel> detectorAssemblies;
    QMap<QString, AssemblyModel> probeAssemblies;
    vtkNew<vtkToneMappingPass> toneMapping;
    vtkNew<vtkOpenGLRenderer> renderer;
    vtkNew<vtkGenericOpenGLRenderWindow> window;
    QSet<int> lutLabelSet;
    QVector<int> leftLabelVec, rightLabelVec;
    bool isInited{ false };
    std::mutex lutMutex;
};

#define brainUtility CBrainAreaUtility::getInstance()

END_NX_NAMESPACE
#endif

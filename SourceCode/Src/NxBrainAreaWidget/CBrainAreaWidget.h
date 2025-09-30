#ifndef __C_BRAIN_AREA_WIDGET_H
#define __C_BRAIN_AREA_WIDGET_H
#include "nxbrainareawidget_global.h"
#include <QWidget>
#include <QMap>
#include <QVector3D>
#include <memory>

class QStandardItem;

namespace Ui {
    class CBrainAreaWidget;
}

class QVTKOpenGLNativeWidget;
class QStandardItemModel;
class vtkGenericOpenGLRenderWindow;
class vtkObject;
class QTimer;
BEGIN_NX_NAMESPACE
    struct BrainAreaInfo;

    class CBrainAreaUtility;

class NX_BRAIN_AREA_WIDGET_EXPORT CBrainAreaWidget: public QWidget
{
    Q_OBJECT
public:
    CBrainAreaWidget(QWidget* parent = nullptr);
    ~CBrainAreaWidget() override;
    void setSignals() const;
    bool addDetectorAndProbes(const QString& tag, const BrainAreaInfo& detectorPos, const BrainAreaInfo& probePos,const QString& detectorType_) const;
    void removeDetectorAndProbe(const QString& tag);
    static void initBrainModel();

signals:
    void signalVtkInited();
    void signalUpdateDetectorTransform(QString planIndex,QVector3D translate, QVector3D rotate);
    void signalUpdateProbeTransform(QString planIndex, QVector3D translate, QVector3D rotate);

public slots:
    void slotRoughnessChanged(int val);
    void slotMetallicChanged(int val);
    void slotExposureChanged(int val);
    void slotAreaChanged(QStandardItem* item);
    void slotOpacityChanged(int index);
    void slotUpdateDetectorTransform();
    void slotPlanIndexesChanged(QString text);
    void slotIsProbeChanged(int index);
    void slotSetLeftHemisphereVisible(bool);
    void slotSetRightHemisphereVisible(bool);
    void slotSetDetectorVisible(bool);
    void slotSetProbeVisible(bool);
    void setAxesVisible(bool);
private:
    void initVtk();
    void initAtlasTree();
private:
    std::unique_ptr<Ui::CBrainAreaWidget> ui;
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkGenericOpenGLRenderWindow* renderWindow;
    QTimer* initTimer;
    QMovie* movie;
    QStandardItemModel* atlasModel;
    QMap<QString, int> treeItemLabelMap;
};

END_NX_NAMESPACE
#endif
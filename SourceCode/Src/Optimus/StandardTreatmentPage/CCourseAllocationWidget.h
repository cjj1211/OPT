#ifndef CCOURSEALLOCATIONWIDGET_H
#define CCOURSEALLOCATIONWIDGET_H
#include "../Global.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class CCourseAllocationWidget; }
QT_END_NAMESPACE
BEGIN_NX_NAMESPACE
class   CStandardTreatmentService;
struct StandardTreatment;
class CCourseAllocationWidget : public QWidget
{
    Q_OBJECT

public:
    CCourseAllocationWidget(QWidget *parent = nullptr);
    ~CCourseAllocationWidget();
    void initWidget();
    void setAction(QString  action);
    void updateAction(StandardTreatment &currentStandardTreatment);
    void setPatientInfo(QString CurrentPatientUid, int CurrentDiseaseName, int CurrentStaging);
    void setTitle(const QString& title);
public: 
    QString currentPatientUid;
    int currentDiseaseName;
    int currentStaging;
    bool isStandStreatment=true;
    
private:
    void connectSignal();
    void actionIsUpdate();
    void clickIsUpdate();
    void directionIsUpdate();
    void wordIsUpdate();
    void softwareIsUpdate();
    void isUpdate();
private slots:
    void slotActionSignalDurationValueChanged(int value);

    void slotActionCountsDurationValueChanged(int value);

    void sloactionTimeDruantionValueChanged(int value);

    void slotClickDurationValueChanged(int value);

    void slotClickCountsDuratiValueChanged(int value);

    void slotDirectionSignalDuratiValueChanged(int value);

    void slotDirectionCountsDuratiValueChanged(int value);

    void slotDirectionTimeDruantiValueChanged(int value);

    void slotWordSignalDuratiValueChanged(int value);

    void slotWordCountsDuratiValueChanged(int value);

    void slotWordTimeDruantiValueChanged(int value);

    void slotSoftWareSignalDuratiValueChanged(int value);

    void slotSoftWareCountsDuratiValueChanged(int value);

    void slotSoftWareTimeDruantiValueChanged(int value);

    void slotActionMinTextChanged(QString value);

    void slotActionMaxTextChanged(QString value);

    void slotClickDiffMinTextChanged(QString value);

    void slotClickDiffMaxTextChanged(QString value);

    void slotClickMinTextChanged(QString value);

    void slotClickMaxTextChanged(QString value);

    void slotDirectionMinTextChanged(QString value);

    void slotDirectionMaxTextChanged(QString value);

    void slotWordMinTextChanged(QString value);

    void slotWordMaxTextChanged(QString value);

    void slotSoftWareMinTextChanged(QString value);

    void slotSoftWareMaxTextChanged(QString value);

    void slotActionCheackStateChanged();

    void slotClickCheackStateChanged();

    void slotDirectionCheackStateChanged();

    void slotWordCheackStateChanged();

    void slotSoftWareCheackStateChanged();

    void slotLeft1CheackStateChanged();

    void slotLeft2CheackStateChanged();
    
    void slotLeft3CheackStateChanged();
    
    void slotLeft4CheackStateChanged();
    
    void slotLeft5CheackStateChanged();
    
    void slotLeft6CheackStateChanged();
    
    void slotLeft7CheackStateChanged();
    
    void slotLeft8CheackStateChanged();
    
    void slotLeft9CheackStateChanged();
    
    void slotRight1CheackStateChanged();
    
    void slotRight2CheackStateChanged();
    
    void slotRight3CheackStateChanged();
    
    void slotRight4CheackStateChanged();
    
    void slotRight5CheackStateChanged();
    
    void slotRight6CheackStateChanged();
    
    void slotRight7CheackStateChanged();
    
    void slotRight8CheackStateChanged();
   
    void slotRight9CheackStateChanged();

    void slotSaveStandardTreatment();

    void slotResetStandardTreatment();

private:
    Ui::CCourseAllocationWidget *ui;
    CStandardTreatmentService* standardTreatmentService;
    std::vector<StandardTreatment>standardTreatment;
    bool actionUpdate = false;
    bool clickUpdate = false;
    bool directionUpdate = false;
    bool wordUpdate = false;
    bool softwareUpdate = false;
};
END_NX_NAMESPACE
#endif // CCOURSEALLOCATIONWIDGET_H

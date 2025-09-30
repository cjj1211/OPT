#include "CTreatmentCfg.h"
#include "Optimus/StandardTreatmentPage/CCourseAllocationWidget.h"
#include "NxAppService/CStandardTreatmentService.h"
BEGIN_NX_NAMESPACE

CTreatmentCfg::CTreatmentCfg(QWidget* parent)
    :QWidget(parent),
    courseAllocationWidget(new CCourseAllocationWidget(this))
{
    ui = std::make_unique<Ui::TreatmentCfg>();
    ui->setupUi(this);
    ui->verticalLayout->addWidget(courseAllocationWidget);
}

CTreatmentCfg::~CTreatmentCfg()
{
}
void CTreatmentCfg::setPatientInfo(QString CurrentPatientUid, int CurrentDiseaseName, int CurrentStaging)
{
    currentPatientUid = CurrentPatientUid;
    currentDiseaseName = CurrentDiseaseName;
    currentStaging = CurrentStaging;
}
void CTreatmentCfg::initWidget()
{
    courseAllocationWidget->setPatientInfo(currentPatientUid, currentDiseaseName, currentStaging);
    courseAllocationWidget->isStandStreatment = false;
    courseAllocationWidget->initWidget();
    courseAllocationWidget->setTitle(tr("Course adjustment"));

}

END_NX_NAMESPACE


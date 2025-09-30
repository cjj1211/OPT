/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __StandardTreatment_SERVICE_H
#define __StandardTreatment_SERVICE_H

#include "nxappservice_global.h"
#include "IAppService.h"
#include <QObject>

BEGIN_NX_NAMESPACE

class CStandardTreatmentRepository;
struct StandardTreatment;
class NX_APP_SERVICE_EXPORT CStandardTreatmentService
    : public IAppService
{
    Q_OBJECT
public:
    CStandardTreatmentService(const QString& patientUid,QObject* parent = nullptr);
    virtual ~CStandardTreatmentService() override;
    std::vector<StandardTreatment> getStandardTreatment(int diseasename, int staging);
    std::vector< StandardTreatment> getStandardTreatmentByPatientFK();
    void deleteStandardTreatment();
    QString currentPatientUid;
    void updateStandardTreatment(StandardTreatment standardTreatment);

private:
    std::unique_ptr<CStandardTreatmentRepository> standardTreatmentRepository;

    int currentStaging;
    int currentDiseasename;


};

END_NX_NAMESPACE

#endif
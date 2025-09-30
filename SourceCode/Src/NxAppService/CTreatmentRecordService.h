/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __CHANNEL_PLAN_SERVICE_H
#define __CHANNEL_PLAN_SERVICE_H

#include "nxappservice_global.h"
#include "IAppService.h"
#include <QObject>

BEGIN_NX_NAMESPACE

class CTreatmentRecordRepository;
struct TreatmentRecord;
class NX_APP_SERVICE_EXPORT CTreatmentRecordService
    : public IAppService
{
    Q_OBJECT
public:
    CTreatmentRecordService(const QString& patientUid, QObject* parent = nullptr);
    virtual ~CTreatmentRecordService() override;
    std::vector<TreatmentRecord> getTreatmentRecordByUpdateTime() const;
    std::vector<TreatmentRecord> getTreatments() const;
    bool deleteTreatmentByUid(const QString& uid) const;
    bool updateTreatmentRecord(const TreatmentRecord& record) const;
private:
    std::unique_ptr<CTreatmentRecordRepository> treatmentRepository ;
    QString currentPatientUid;
};

END_NX_NAMESPACE

#endif
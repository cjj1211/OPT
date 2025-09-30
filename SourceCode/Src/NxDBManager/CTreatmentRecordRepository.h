/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TREATMENTRECORD_REPOSITORY_H
#define __C_TREATMENTRECORD_REPOSITORY_H

#include "nxdbmanager_global.h"
#include "IRepository.h"

BEGIN_NX_NAMESPACE
struct TreatmentRecord;
class NXDBMANAGER_EXPORT CTreatmentRecordRepository : public IRepository<TreatmentRecord>
{
public:
    CTreatmentRecordRepository();
    virtual ~CTreatmentRecordRepository() override;

    
    /**

        @fn     getTreatmentRecordByPatient
        @brief  Get treatment record by patient
        @param  patientFK     - 
        @param  StartDatetime - 传空则不查开始时间
        @param  trainType     - 传-1则不查游戏类型
        @retval               - treatment record by patient
        @author CuiJunJie
        @date   15.09.2023

    **/
    std::vector<TreatmentRecord> getTreatmentRecordByPatient( const QString& patientFK,const QString &StartDatetime,int trainType);

    std::vector<TreatmentRecord> getTreatmentRecordByUpateTime( );



};

END_NX_NAMESPACE

#endif

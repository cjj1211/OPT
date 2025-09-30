/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#include "CPatientRepository.h"

#include "NxEntity/ChannelName.h"
#include "NxEntity/ChannelPlan.h"
#include "NxEntity/Patient.h"
#include "NxEntity/IdentityInfo.h"
#include "NxEntity/TrainEvents.h"
#include "NxEntity/TreatmentCfg.h"
#include "NxEntity/TreatmentRecord.h"
BEGIN_NX_NAMESPACE
    CPatientRepository::CPatientRepository()
    :IRepository()
{
}

CPatientRepository::~CPatientRepository()
{
}

bool CPatientRepository::createPatient(const IdentityInfo& identityInfo, const Patient& patient) {
    
    db->begin();
    if (!(db->insert<IdentityInfo>(identityInfo) == 1))
    {
        LOG_ERR("Insert identityInfo fail, data is rollbacked.");
        db->rollback();
        return false;
    }
    auto patientToStore = patient;
    patientToStore.IdentifyFK = identityInfo.UID;
    if (!(db->insert<Patient>(patientToStore))) {
        LOG_ERR("Insert patient fail, data is rollbacked.");
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

bool CPatientRepository::updatePatient(const IdentityInfo& identityInfo, const Patient& patient)
{
    db->begin();
    if (!(db->update<IdentityInfo>(identityInfo) == 1))
    {
        LOG_ERR("Insert identityInfo fail, data is rollbacked.");
        db->rollback();
        return false;
    }
    auto patientToStore = patient;
    patientToStore.IdentifyFK = identityInfo.UID;
    if (!(db->update<Patient>(patientToStore))) {
        LOG_ERR("Insert patient fail, data is rollbacked.");
        db->rollback();
        return false;
    }
    db->commit();
    return true;
}

int CPatientRepository::getPatientCount(QString filter)
{
    QString sql = "IsDeleted = 0 ";
    if (!filter.isEmpty()) {
        sql += " AND (PatientName REGEXP '" + filter + "' OR CaseID REGEXP '" + filter + "') ";
    }
    return static_cast<int>(db->query<Patient>(sql.toStdString()).size());
}

Patient CPatientRepository::getPatientByIdentifyFK(const QString& identifyFK)
{
    auto patient = IRepository::db->query<Patient>(" IdentifyFK='" + identifyFK.toStdString() + "' and IsDeleted = 0");
    if (patient.empty())
    {
        return Patient();
    }
    else {
        return patient[0];
    }
}

std::vector<Patient> CPatientRepository::getPatientByPatientName(const QString& patientName)
{
    auto patient = IRepository::db->query<Patient>(" PatientName='" + patientName.toStdString()+"'" + " and IsDeleted = 0");
    if (patient.empty())
    {
        return std::vector<Patient>();
    }
    else {
        return patient;
    }
}

bool CPatientRepository::getPatientByCaseId(const QString& caseId, Patient& patient)
{
    auto patientList = IRepository::db->query<Patient>(" CaseId='" + caseId.toStdString()+"'" + " and IsDeleted = 0");
    if (patientList.empty())
    {
        return false;
    }
    else {
        patient = patientList[0];
        return true;
    }
}

std::vector<Patient> CPatientRepository::getPatientByGender(GenderEmu gender)
{
    int gend= gender;
    auto patient = IRepository::db->query<Patient>(" Gender=" +QString::number(gend).toStdString() + " and IsDeleted = 0");
    if (patient.empty())
    {
        return std::vector<Patient>();
    }
    else {
        return patient;
    }
}

std::vector<Patient> CPatientRepository::getPatientByBirthday(const QString& birthday)
{
    auto patient = IRepository::db->query<Patient>(" Birthday='" + birthday.toStdString()+"'" + " and IsDeleted = 0");
    if (patient.empty())
    {
        return std::vector<Patient>();
    }
    else {
        return patient;
    }
}

std::vector<Patient> CPatientRepository::getPatientCurrentPage(int currentpage, int number, QString filter)
{
    QString where = " WHERE IsDeleted = 0 ";
    if (!filter.isEmpty())
    {
        where += " AND (PatientName REGEXP '" + filter + "' OR CaseID REGEXP '" + filter + "') ";
    }
    QString sql = where + QString(" order by UpdateDateTime  desc limit %1 offset %2  ").arg(QString::number(number)).arg(QString::number((currentpage - 1) * number));

    auto patient = IRepository::db->query<Patient>(sql.toStdString());
    if (patient.empty())
    {
        return std::vector<Patient>();
    }
    else {
        return patient;
    }
}

bool CPatientRepository::isNameDuplicate(const QString& patientName, const QString& uid)
{
    auto patient = IRepository::db->query<Patient>(" PatientName='" + patientName.toStdString() + "'" 
        + " and UID!='" + uid.toStdString() + "'"
        + " and IsDeleted = 0");
    if (patient.empty())
    {
        return false;
    }
    else {
        return true;
    }
}

bool CPatientRepository::isCaseIdDuplicate(const QString& caseId, const QString& uid)
{
    auto patient = IRepository::db->query<Patient>(" CaseID='" + caseId.toStdString() + "'"
        + " and UID!='" + uid.toStdString() + "'"
        + " and IsDeleted = 0");
    if (patient.empty())
    {
        return false;
    }
    else {
        return true;
    }
}

bool CPatientRepository::deleteWithRefInfoByUid(const QString& uid)
{
    db->begin();

    auto patients = db->query<Patient>(" UID='" + uid.toStdString() + "' and IsDeleted=0");
    if(patients.empty())
    {
        db->rollback();
        return false;
    }
    auto patient = patients[0];

    auto identityInfos = db->query<IdentityInfo>(" UID='" + patient.IdentifyFK + "' and IsDeleted=0");
    if(identityInfos.empty())
    {
        db->rollback();
        return false;
    }
    identityInfos[0].IsDeleted = true;
    if(!db->update<IdentityInfo>(identityInfos[0]))
    {
        db->rollback();
        return false;
    }

    const auto channelPlans = db->query<ChannelPlan>(" PatientFK='" + patient.UID + "' and IsDeleted=0");
    for(auto channelPlan : channelPlans)
    {
        channelPlan.IsDeleted = true;
        if(!db->update<ChannelPlan>(channelPlan))
        {
            db->rollback();
            return false;
        }
    }

    const auto channelNames = db->query<ChannelName>(" PatientFK='" + patient.UID + "' and IsDeleted=0");
    for(auto channelName : channelNames)
    {
        channelName.IsDeleted = true;
        if(!db->update<ChannelName>(channelName))
        {
            db->rollback();
            return false;
        }
    }

    const auto treatmentCfgs = db->query<TreatmentCfg>(" PatientFK='" + patient.UID + "' and IsDeleted=0");
    for(auto treatmentCfg : treatmentCfgs)
    {
        treatmentCfg.IsDeleted = true;
        if(!db->update<TreatmentCfg>(treatmentCfg))
        {
            db->rollback();
            return false;
        }
    }

    const auto treatmentRecords = db->query<TreatmentRecord>(" PatientFK='" + patient.UID + "' and IsDeleted=0");
    for(auto treatmentRecord: treatmentRecords)
    {
        auto trainEvents = db->query<TrainEvents>(" TreatmentRecordFK='" + treatmentRecord.UID + "' and IsDeleted=0");
        for(auto trainEvent : trainEvents)
        {
            trainEvent.IsDeleted = true;
            if(!db->update<TrainEvents>(trainEvent))
            {
                db->rollback();
                return false;
            }
        }

        treatmentRecord.IsDeleted = true;
        if(!db->update<TreatmentRecord>(treatmentRecord))
        {
            db->rollback();
            return false;
        }
    }

    patient.IsDeleted = true;
    if(!db->update<Patient>(patient))
    {
        db->rollback();
        return false;
    }

    db->commit();

    return true;
}


END_NX_NAMESPACE



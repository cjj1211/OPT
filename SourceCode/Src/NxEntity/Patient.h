#/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __PATIENT_H
#define __PATIENT_H

#include "nxentity_global.h"
#include "EntityBase.h"
#include <string>
#include <iguana/reflection.hpp>
#include <ormpp/dbng.hpp>
#include"Gender.h"
BEGIN_NX_NAMESPACE

using namespace ormpp;

enum TypeOfIllness {
    UNKOWN = 0,
    ALS,        // 渐冻症
    Seizures,   // 癫痫
    Depression, // 抑郁症
    SCI,        // 脊髓损伤
    PD,         // 帕金森
    Blindness   // 失明
};

struct Patient :public EntityBase {
    std::string IdentifyFK;  // 认证信息表外键
    std::string PatientName;
    std::string CaseID;  // 唯一
    std::string DeviceID;
    char Gender;  // 枚举值
    char IllType; // 病情
    char IllStage; // 分期 
    std::string Birthday;  // 日期类型
    std::string Note;  // TEXT 类型
    std::string DiseaseCourse;
    char Sensitivity{9}; // 波形灵敏度
    char PaperSpeed{3}; // 走纸速度

    int getSensitivityIndex() const { return Sensitivity; }
    void setSensitivityIndex(int index) { Sensitivity = index; }
    int getPaperSpeedIndex() const { return PaperSpeed; }
    void setPaperSpeedIndex(int index) { PaperSpeed = index; }

    int getAge() {
        auto birthday = QString::fromStdString(Birthday).split(' ')[0];
        auto age = QDate::currentDate().year() - QDate::fromString(birthday, "yyyy-MM-dd").year();
        return age;
    }
};
REFLECTION(Patient, UID, IdentifyFK, PatientName, CaseID, DeviceID, Gender, IllType, IllStage,
    Birthday, Note, DiseaseCourse, UpdateDateTime, IsDeleted, CreateDateTime, Sensitivity, PaperSpeed);

END_NX_NAMESPACE
#endif
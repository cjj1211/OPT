/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 崔俊杰 junjie.cui@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#include "CTrainEventsRepository.h"

BEGIN_NX_NAMESPACE

CTrainEventsRepository::CTrainEventsRepository()
    :IRepository()
{
}

CTrainEventsRepository::~CTrainEventsRepository()
{
}

std::vector<TrainEvents> CTrainEventsRepository::getTrainEventsByTreatmentRecordFK(const QString& treatmentRecordFK)
{



    auto trainEvents = IRepository::db->query<TrainEvents>("TreatmentRecordFK= '"+ treatmentRecordFK.toStdString() + "'" + " and IsDeleted = 0");
    if (trainEvents.empty())
    {
        return std::vector<TrainEvents>();
    }
    else {
        return trainEvents;
    }
}

std::vector<TrainEvents> CTrainEventsRepository::getTrainEventsByEventDateTime(const QString& eventDateTime)
{
    auto trainEvents = IRepository::db->query<TrainEvents>("EventDateTime= '" + eventDateTime.toStdString() + "'" + " and IsDeleted = 0");
    if (trainEvents.empty())
    {
        return std::vector<TrainEvents>();
    }
    else {
        return trainEvents;
    }
}


END_NX_NAMESPACE



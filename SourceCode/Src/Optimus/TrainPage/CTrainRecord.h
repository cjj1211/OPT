/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TRAIN_RECORD_H
#define __C_TRAIN_RECORD_H
#include "../Global.h"
#include "ui_TrainRecord.h"
#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
class CTrainRecordComponent;
class CTreatmentRecordService;
class CDetect;
class CTrainRecord : public QWidget
{
    Q_OBJECT
        QT_TRANSLATE_HANDLER
public:
    CTrainRecord(const QString& currentPatientUid, QWidget* parent = nullptr);
    ~CTrainRecord() override;

public slots:
    void slotJumpToPlayback(const QString& recordUid);
    void slotExitPlayback();
private:
    std::unique_ptr<Ui::TrainRecord> ui;
    CTrainRecordComponent* record;
    CDetect* playback;
    CTreatmentRecordService* service;

    QString patientUid;
};

END_NX_NAMESPACE
#endif
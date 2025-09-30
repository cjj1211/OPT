/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_TRAIN_RECORD_COMPONENT_H
#define __C_TRAIN_RECORD_COMPONENT_H
#include "Global.h"
#include "ui_CTrainRecordComponent.h"
#include "NxEntity/TreatmentRecord.h"
#include <QWidget>
#include <memory>

BEGIN_NX_NAMESPACE
class CTreatmentRecordService;
class CTrainRecordComponent : public QWidget
{
    Q_OBJECT

public:
    CTrainRecordComponent(CTreatmentRecordService* treatmentRecordService,QWidget *parent = nullptr);
    ~CTrainRecordComponent() override;

signals:
    void signalJumpToPlayback(const QString& recordUid);

private slots:
    void slotRowSelected();
    void slotPlaybackClicked();
    void slotRecordSelected();
    void slotAnalysisClicked();
    void slotDeleteClicked();
    void slotCellChanged(int row, int col) ;

private:
    std::unique_ptr<Ui::CTrainRecordComponent> ui;
    CTreatmentRecordService* service;
    QVector<TreatmentRecord> records;
};

END_NX_NAMESPACE
#endif
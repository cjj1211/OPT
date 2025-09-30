/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DETECT_TOOLBAR_H
#define __C_DETECT_TOOLBAR_H
#include "Global.h"
#include <QWidget>
#include "ui_CDetectToolbar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CDetectToolbarClass; };
QT_END_NAMESPACE

BEGIN_NX_NAMESPACE

class CDetectToolbar : public QWidget
{
    Q_OBJECT

public:
    CDetectToolbar(QWidget *parent = nullptr);
    ~CDetectToolbar();

public slots:
    void slotRun(bool);
    void slotRecord(bool);
    void slotUpdateRuntime(const uint64_t);

signals:
    void signalRun(bool);
    void signalRecord(bool);
    void signalImpedance();
    void signalChannelMap();
    void signalSpikeScope();
private:
    Ui::CDetectToolbarClass *ui;
};

END_NX_NAMESPACE
#endif
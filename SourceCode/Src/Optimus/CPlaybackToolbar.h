/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_PLAYBACK_TOOLBAR_H
#define __C_PLAYBACK_TOOLBAR_H
#include "Global.h"
#include <QWidget>
#include "ui_CPlaybackToolbar.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CPlaybackToolbarClass; };
QT_END_NAMESPACE

BEGIN_NX_NAMESPACE
class CPlaybackToolbar : public QWidget
{
    Q_OBJECT
    QT_TRANSLATE_HANDLER
public:
    CPlaybackToolbar(QWidget *parent = nullptr);
    ~CPlaybackToolbar() override;

    void setTimeLength(const int timeLenInSecs) const;
    void jumpByEvent(const int timeLenInSecs);

public slots:
    void slotUpdateRuntime(const uint64_t runTimeInSecs);
    void slotSliderChanged(int runtime);
    void slotPlayBtnToggled(bool isRun);
    void slotJumpTo();
    void slotServerStoped();
    void slotSliderPressed();

signals:
    void signalPlay(bool);
    void signalImpedance();
    void signalExport();
    void signalJumpTo(const uint64_t runtimeInSecs);
    void signalBack();
    void signalSpikeScope();

private:
    Ui::CPlaybackToolbarClass *ui;
    bool sliderPressed;
};
END_NX_NAMESPACE
#endif
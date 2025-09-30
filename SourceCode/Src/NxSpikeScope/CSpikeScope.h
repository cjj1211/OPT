/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期: September 2023
 *  修改记录:
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef C_NX_SPIKE_SCOPE_H
#define C_NX_SPIKE_SCOPE_H

#include "NxSpikeScopeGlobal.h"
#include "NxCommonWidget/CFramelessWidget.h"
#include <QDialog>
#include <memory>


class CSpikePlot;

namespace Ui
{
    class CSpikeScope;
}

class NX_SPIKE_SCOPE_EXPORT CSpikeScope final: public FramelessWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CSpikeScope)

public:
    CSpikeScope(const QString& channelName_, const int sampleRate_, QWidget* parent = nullptr);
    ~CSpikeScope() override;

    CSpikeScope(CSpikeScope&&) = delete;
    CSpikeScope& operator= (CSpikeScope&&) = delete;

    void setWaveformName(const QString& channelName, const int sampleRate_) ;
    void updateWaveforms(const QVector<float>& waveformSegment) const;

signals:
    void signalCloseSpikeScope();

private slots:
    void slotUpdateTimeScale( int index_) ; // ms 毫秒
    void slotSetMaxSpikeCount(const int index);
    void clearSpikes() const;
    bool isEnabledDrag(QMouseEvent* e) override;

private:
    CSpikePlot* spikePlot;
    std::unique_ptr<Ui::CSpikeScope>  ui;
};

#endif
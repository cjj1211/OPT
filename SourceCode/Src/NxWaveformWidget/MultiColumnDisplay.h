/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_MULTI_COLUMN_DISPLAY_H
#define __C_MULTI_COLUMN_DISPLAY_H
#include "nxwaveformwidget_global.h"
#include "waveformdisplaymanager.h"
#include "waveformdisplaycolumn.h"
#include <QtWidgets>

BEGIN_NX_NAMESPACE

class NX_WAVEFORM_WIDGET_EXPORT MultiColumnDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit MultiColumnDisplay( QWidget *parent = nullptr);
    ~MultiColumnDisplay() override;

    bool addColumn();

    void setWaveformWidth(int width);

    void reset();

    void enableSelectedChannels(bool enabled);

    void updateForRescan();

    void addWaveforms();

    QStringList getPinnedWaveNames() const { return displayColumn->getPinnedWaveNames(); }
    void setPinnedWaveforms(const QStringList& pinnedWaveNames) { displayColumn->setPinnedWaveforms(pinnedWaveNames); }
    bool arePinnedShown() const { return displayColumn->arePinnedShown(); }
    void setShowPinned(const bool showPinned);

    void updateRenderWaveName();

signals:
    void signalUpdateDisplayParam();
    void signalUpdateRunTime(const QString& runtime);
    void signalToggleSelectedWaveforms();
    void signalUpdateSelectedWavename(QString nativeName);

public slots:
    void updateFromState();
    void slotUpdateDisplayWaveNames();
    void loadWaveformData(UiDisplayDto& uiDisplayDto);

private: 
    WaveformDisplayManager* waveformManager; 
    WaveformDisplayColumn* displayColumn;
    
    QShortcut* minusYScaleShortcut;
    QShortcut* plusYScaleShortcut;

    //static const int MaxNumColumns = 10;
    static const int MaxNumColumns = 16;


    int tPaperSpeedIndex;
    bool rollModeFormerValue;

    void updateColumnIndices();
    void updateLayout();
};

END_NX_NAMESPACE

#endif


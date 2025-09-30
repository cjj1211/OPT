#ifndef __C_WAVEFORM_DISPLAY_H
#define __C_WAVEFORM_DISPLAY_H

#include "MultiWaveformPlot.h"
#include <QtWidgets>

BEGIN_NX_NAMESPACE

class MultiColumnDisplay;
class WaveformDisplayColumn : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformDisplayColumn(int columnIndex_, WaveformDisplayManager* waveformManager,
                                   MultiColumnDisplay *parent_);
    ~WaveformDisplayColumn();

    void setIndex(int columnIndex_) { columnIndex = columnIndex_; waveformPlot->setIndex(columnIndex_); }

    bool isColumnVisible() const { return visible; }

    void setWaveformWidth(int width);
    void updateNow() const { waveformPlot->updateNow(); }

    inline void loadWaveformData(UiDisplayDto& uiDisplayDto) const
    {
	    waveformPlot->loadWaveformData(uiDisplayDto);
    }

    QStringList getPinnedWaveNames() const { return waveformPlot->getPinnedWaveNames(); }
    QStringList getRenderWaveNames() const { return waveformPlot->getRenderWaveNames(); }
    void updateVisibleWaveName() const;
    void setPinnedWaveforms(const QStringList& pinnedWaveNames) const { waveformPlot->setPinnedWaveforms(pinnedWaveNames); }
    bool arePinnedShown() const { return pinnedShown; }
    void setShowPinnedWaveforms(bool show);

    void enableSelectedWaveforms(bool enable) const { waveformPlot->enableSelectedWaveforms(enable); }

public slots:
    void showPinnedWaveformsSlot();
    void updateFromState();

signals:
    void signalUpdateDisplayParam();
    void signalDispalyWaveNamesChanged();
    void signalToggleSelectedWaveforms();
    void signalUpdateSelectedWavename(QString nativeName);
private:
    WaveformDisplayColumn(const WaveformDisplayColumn&);  // Copying not allowed.
    WaveformDisplayColumn& operator=(const WaveformDisplayColumn&) {};  // Copying not allowed.
    MultiColumnDisplay* parent;
    int columnIndex;
    bool visible;
    bool pinnedShown;

    MultiWaveformPlot* waveformPlot;

    QToolButton* lessSpacingButton;
    QToolButton* moreSpacingButton;
    QCheckBox* showPinnedCheckBox;
    QSpacerItem* verticalSpacer;
};
END_NX_NAMESPACE
#endif

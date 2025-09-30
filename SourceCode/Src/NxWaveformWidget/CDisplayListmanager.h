/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_DISPLAY_LIST_MANAGER_H
#define __C_DISPLAY_LIST_MANAGER_H
#include "nxwaveformwidget_global.h"
#include "DisplayedWaveform.h"
#include <QObject>

BEGIN_NX_NAMESPACE

struct WaveIndex {
    int index;
    bool inPinned;
};

class CDisplayListManager: public QObject
{
    Q_OBJECT
public:
    CDisplayListManager(QList<DisplayedWaveform>& displayList_, QList<DisplayedWaveform>& pinnedList_, QObject* parent = nullptr);
    ~CDisplayListManager() override;
    void populateDisplayList(const QStringList& displayWaveforms);
    void addPinnedWaveform(const QString& waveName);
    void pinSelectedWaveforms();
    void unpinSelectedWaveforms() const;
    void unpinAllWaveforms();
    int findSelectedWaveform(const QList<DisplayedWaveform>& list, int y) const;

    DisplayedWaveform* displayedWaveform(WaveIndex waveIndex);
    DisplayedWaveform* displayedWaveform(int index, bool inPinned);

    int numDisplayedWaveforms(bool inPinned = false) const;
    int numSelectedWaveforms(bool inPinned = false) const;
    int numSelectedAmplifierChannels(int numFiltersDisplayed, bool arrangeByFilter) const;
    int numSelectedVisibleAmplifierChannels(int numFiltersDisplayed, bool arrangeByFilter) const;
    int sectionIDOfSelectedWaveforms() const;
    bool selectedWaveformsAreAmplifiers() const;

    void enableSelectedWaveforms(bool enable);
    void toggleSelectedWaveforms();

    bool isValidDragTarget(WaveIndex target, int numFiltersDisplayed, bool arrangeByFilter) const;

    bool moveSelectedWaveforms(QList<DisplayedWaveform> &list, int position);
    bool moveSelectedWaveformsArrangedByFilter(QList<DisplayedWaveform> &list, int position, int numFilters);
    int numWaveformsInFirstSection(const QList<DisplayedWaveform> &list) const;

    int calculateYCoords(QList<DisplayedWaveform>& list, int y, int labelHeight, double zoomFactor, int& minSpacing);
    void addYOffset(QList<DisplayedWaveform>& list, int yOffset);

    void selectSingleWaveform(WaveIndex waveIndex);
    bool selectNextWaveform() { return selectNextOrPreviousWaveform(true); }
    bool selectPreviousWaveform() { return selectNextOrPreviousWaveform(false); }

    std::vector<bool> selectionRecord() const;
    bool selectionRecordsAreEqual(const std::vector<bool>& a, const std::vector<bool>& b) const;

    // These functions update the system state intelligently, and automatically trigger a state change if anything changed.
    void updateOrderInState(int numFiltersDisplayed, bool arrangeByFilter);

signals:
    void signalUpdateSelectedWavename(QString nativeName);

private:
    QList<DisplayedWaveform>& displayList;
    QList<DisplayedWaveform>& pinnedList;

    void cleanUpDisplayList();

    DisplayedWaveform displayedWaveformFromName(const QString& waveName) const;
    DisplayedWaveform waveformDivider() const { return DisplayedWaveform("", WaveformType::WaveformDivider, nullptr); }

    bool selectedWaveformsAreEnabled() const;

    // Must manually trigger a state change after calling these functions.
    void deselectAllWaveforms();
    void selectWaveform(const QString& nativeName, bool select = true);
    bool selectNextOrPreviousWaveform(bool selectNext);
};

END_NX_NAMESPACE
#endif
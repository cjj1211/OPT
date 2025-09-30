#include <iostream>
#include <limits>
#include "CDisplayStatus.h"
#include "CDisplayListManager.h"

CDisplayListManager::CDisplayListManager( QList<DisplayedWaveform>& displayList_, QList<DisplayedWaveform>& pinnedList_, QObject* parent) :
    QObject(parent),
	displayList(displayList_),
    pinnedList(pinnedList_)
{
}

CDisplayListManager::~CDisplayListManager()
{
}

void CDisplayListManager::populateDisplayList(const QStringList& displayWaveforms)
{
    displayList.clear();

    for (int i = 0; i < displayWaveforms.size(); ++i) {
        DisplayedWaveform wave = displayedWaveformFromName(displayWaveforms.at(i));
        displayList.append(wave);
    }

    cleanUpDisplayList();  // Remove redundant waveform dividers and calculate display regions.
}

DisplayedWaveform CDisplayListManager::displayedWaveformFromName(const QString& waveName) const
{
    if (waveName == "/") return waveformDivider();

    UiChannel* channel = signalManager.channelByName(waveName);//recorderService->signalSources->channelByName(waveName);
    if (!channel)
    {
        return DisplayedWaveform( "", WaveformType::UnknownWaveform, nullptr);
    }
    WaveformType waveType = DisplayedWaveform::translateSignalTypeToWaveformType(channel->getSignalType());
    DisplayedWaveform dw(waveName, waveType, channel);
    return dw;
}

void CDisplayListManager::cleanUpDisplayList()
{
    if (displayList.isEmpty()) return;

    // Remove waveform dividers at beginning or end of list.
    while (displayList.first().isDivider()) {
        displayList.removeFirst();
        if (displayList.isEmpty()) return;
    }
    while (displayList.last().isDivider()) {
        displayList.removeLast();
        if (displayList.isEmpty()) return;
    }

    // Remove duplicate waveform dividers.
    for (int i = 1; i < displayList.size() - 1; ++i) {
        if (displayList.at(i).isDivider() && displayList.at(i + 1).isDivider()) {
            displayList.removeAt(i + 1);
            --i;
        }
    }

    // Calculate display sections.
    int section = 1;
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList.at(i).isDivider()) {
            displayList[i].sectionID = DividerSectionID;
            ++section;
        } else {
            displayList[i].sectionID = section;
        }
    }
}

void CDisplayListManager::addPinnedWaveform(const QString& waveName)
{
    UiChannel* channel = signalManager.channelByName(waveName);
    if (channel) {
        pinnedList.append(displayedWaveformFromName(waveName));
    }
}

void CDisplayListManager::pinSelectedWaveforms()
{
    for (const auto& i : displayList)
    {
        if (i.isSelected()) {
            // Don't pin channels if they are disabled and invisible.
            if (i.isEnabled() || displayStatus.isShowDisabledChannels()) {
                addPinnedWaveform(i.waveName);
            }
        }
    }
}

void CDisplayListManager::unpinSelectedWaveforms() const
{
    for (int i = 0; i < pinnedList.size(); ++i) {
        if (pinnedList.at(i).isSelected()) {
            pinnedList.removeAt(i);
            --i;
        }
    }
}

void CDisplayListManager::unpinAllWaveforms()
{
    pinnedList.clear();
}

// If cursor is near a waveform, this function returns the waveform index (an integer between 0 and N for N+1 waveforms).
// If cursor is above waveform 0, this function returns -1.
// If cursor is between waveform N and N+1, this function returns -(N+2).
// If cursor is below waveform N, this function returns -(N+2).
int CDisplayListManager::findSelectedWaveform(const QList<DisplayedWaveform>& list, int y) const
{
    if (list.isEmpty()) return -1;
    int length = list.size();

    for (int i = 0; i < length; ++i) {
        if (y >= list[i].yTop && y <= list[i].yBottom) return i;
    }
    if (y < list[0].yTop) return -1;
    if (y > list[length - 1].yBottom) return -(length + 1);
    for (int i = 0; i < length - 1; ++i) {
        if (y > list[i].yBottom && y < list[i + 1].yTop) return -(i + 2);
    }
    std::cerr << "CDisplayListManager::findSelectedWaveform: This line should never be reached." << '\n';
    return 0;  // This line should never be reached - included so the compiler doesn't complain.
}

DisplayedWaveform* CDisplayListManager::displayedWaveform(WaveIndex waveIndex)
{
    if (waveIndex.inPinned) {
        return &pinnedList[waveIndex.index];
    } else {
        return &displayList[waveIndex.index];
    }
}

DisplayedWaveform* CDisplayListManager::displayedWaveform(int index, bool inPinned)
{
    if (inPinned) {
        return &pinnedList[index];
    } else {
        return &displayList[index];
    }
}

int CDisplayListManager::numDisplayedWaveforms(bool inPinned) const
{
    if (inPinned) {
        return pinnedList.size();
    } else {
        return displayList.size();
    }
}

int CDisplayListManager::numSelectedWaveforms(bool inPinned) const
{
    int numSelected = 0;
    if (inPinned) {
        for (int i = 0; i < pinnedList.size(); ++i) {
            if (pinnedList.at(i).isSelected()) ++numSelected;
        }
    } else {
        for (int i = 0; i < displayList.size(); ++i) {
            if (displayList.at(i).isSelected()) ++numSelected;
        }
    }
    return numSelected;
}

int CDisplayListManager::numSelectedAmplifierChannels(int numFiltersDisplayed, bool arrangeByFilter) const
{
    if (displayList.isEmpty()) return 0;
    if (!displayList[0].isAmplifier()) return 0;

    int numSelected = 0;
    int waveformsInFirstSection = numWaveformsInFirstSection(displayList);
    for (int i = 0; i < waveformsInFirstSection; ++i) {
        if (displayList.at(i).isSelected()) ++numSelected;
    }
    if (!arrangeByFilter) {
        numSelected /= numFiltersDisplayed;
    }
    return numSelected;
}

int CDisplayListManager::numSelectedVisibleAmplifierChannels(int numFiltersDisplayed, bool arrangeByFilter) const
{
    if (displayList.isEmpty()) return 0;
    if (!displayList.at(0).isAmplifier()) return 0;

    bool showDisabled = displayStatus.isShowDisabledChannels();
    int numSelected = 0;
    int waveformsInFirstSection = numWaveformsInFirstSection(displayList);
    for (int i = 0; i < waveformsInFirstSection; ++i) {
        if (displayList.at(i).isSelected()) {
            if (displayList.at(i).isEnabled() || showDisabled) ++numSelected;
        }
    }
    if (!arrangeByFilter) numSelected /= numFiltersDisplayed;
    return numSelected;
}

// This function assumes that all selected waveforms are in the same section, if multiple waveforms are selected.
int CDisplayListManager::sectionIDOfSelectedWaveforms() const
{
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList.at(i).isSelected()) return displayList.at(i).sectionID;
    }
    for (int i = 0; i < pinnedList.size(); ++i) {
        if (pinnedList.at(i).isSelected()) return pinnedList.at(i).sectionID;
    }
    return DividerSectionID;  // No waveforms are selected.
}

// This function assumes that all selected waveforms are the same type, if multiple waveforms are selected.
bool CDisplayListManager::selectedWaveformsAreAmplifiers() const
{
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList.at(i).isSelected()) return displayList.at(i).isAmplifier();
    }
    for (int i = 0; i < pinnedList.size(); ++i) {
        if (pinnedList.at(i).isSelected()) return pinnedList.at(i).isAmplifier();
    }
    return false;
}

// This function assumes that all selected waveforms are enabled or all are disabled, if multiple waveforms are selected.
bool CDisplayListManager::selectedWaveformsAreEnabled() const
{
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList.at(i).isSelected()) return displayList.at(i).isEnabled();
    }
    for (int i = 0; i < pinnedList.size(); ++i) {
        if (pinnedList.at(i).isSelected()) return pinnedList.at(i).isEnabled();
    }
    return false;  // No waveforms are selected.
}

void CDisplayListManager::enableSelectedWaveforms(bool enable)
{
    for (int i = 0; i < pinnedList.size(); ++i) {
        if (pinnedList.at(i).isSelected()) {
            pinnedList[i].setEnabled(enable);
        }
    }
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList.at(i).isSelected()) {
            displayList[i].setEnabled(enable);
        }
    }
}

void CDisplayListManager::toggleSelectedWaveforms()
{
    enableSelectedWaveforms(!selectedWaveformsAreEnabled());
}

// Move all selected waveforms to the spot just after position.  In a list of N waveforms indexed 0 to N-1,
// position can have a value between -1 and N-1.  Use position = -1 to insert at top of list.
bool CDisplayListManager::moveSelectedWaveforms(QList<DisplayedWaveform> &list, int position)
{
    bool realMove = false;
    int target = position;
    for (int i = target; i >= 0; --i) {
        if (list[i].isSelected()) {
            if (i != target) realMove = true;
            list.move(i, target--);
        }
    }
    target = position + 1;
    for (int i = target; i < list.size(); ++i) {
        if (list[i].isSelected()) {
            if (i != target) realMove = true;
            list.move(i, target++);
        }
    }
    return realMove;
}

// Move all selected waveforms to the spot just after position, accounting for multiple sections displaying numFilters
// different filters.
// In a list of N waveforms indexed 0 to N-1, position can have a value between -1 and N-1.  Use position = -1 to insert
// at top of list.
bool CDisplayListManager::moveSelectedWaveformsArrangedByFilter(QList<DisplayedWaveform> &list, int position, int numFilters)
{
    bool realMove = false;

    // First, count how many displayed waveforms are in each filter section.
    int waveformsPerSection = numWaveformsInFirstSection(list);

    while (position >= waveformsPerSection + 1) {   // + 1 to account for section divider
        position -= waveformsPerSection + 1;
    }

    int sectionStart = 0;
    for (int filter = 0; filter < numFilters; ++filter) {
        int target = sectionStart + position;
        for (int i = target; i >= sectionStart; --i) {
            if (list[i].isSelected()) {
                if (i != target) realMove = true;
                list.move(i, target--);
            }
        }
        target = sectionStart + position + 1;
        for (int i = target; i < sectionStart + waveformsPerSection; ++i) {
            if (list[i].isSelected()) {
                if (i != target) realMove = true;
                list.move(i, target++);
            }
        }
        sectionStart += waveformsPerSection + 1;  // + 1 to account for section divider
    }
    return realMove;
}

// Returns number of waveforms in first section.
int CDisplayListManager::numWaveformsInFirstSection(const QList<DisplayedWaveform> &list) const
{
    if (list.isEmpty()) return 0;
    int section = list[0].sectionID;
    int waveformsPerSection = 1;
    for (int i = 1; i < list.size(); ++i) {
        if (list[i].sectionID != section) break;
        else ++waveformsPerSection;
    }
    return waveformsPerSection;
}

bool CDisplayListManager::isValidDragTarget(WaveIndex target, int numFiltersDisplayed, bool arrangeByFilter) const
{
    if (target.inPinned) return true;
    int index = target.index;
    int section = sectionIDOfSelectedWaveforms();
    bool multiFilter = (selectedWaveformsAreAmplifiers()) && numFiltersDisplayed > 1 && arrangeByFilter;

    if (displayList.isEmpty()) return false;

    if (!multiFilter) {  // Simpler case: Dragging waveforms without multiple filters taken into consideration.
        if (index == -1) {  // Before first waveform.
            return (displayList.at(0).sectionID == section);
        }
        if (index < -1) {   // Drop between waveforms...
            const DisplayedWaveform* wave1 = &displayList[-(index + 2)];
            if (index == -displayList.size() - 1) {  // After last waveform.
                return (wave1->sectionID == section);
            } else {
                const DisplayedWaveform* wave2 = &displayList[-(index + 1)];
                if (wave1->sectionID == section || wave2->sectionID == section) {  // ...but only in same section...
                    if (wave1->waveNameWithoutFilter() == wave2->waveNameWithoutFilter()) {  // ...and only between channels.
                        return false;
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }
        } else {            // Drop after selected waveform...
            const DisplayedWaveform* wave1 = &displayList[index];
            if (index == displayList.size() - 1) {  // After last waveform.
                return (wave1->sectionID == section);
            } else {
                const DisplayedWaveform* wave2 = &displayList[index + 1];
                if (wave1->sectionID == section || wave2->sectionID == section) {  // ...but only in same section...
                    if (wave1->waveNameWithoutFilter() == wave2->waveNameWithoutFilter()) {  // ...and only between channels.
                        return false;
                    }  else {
                        return true;
                    }
                } else {
                    return false;
                }
            }
        }
    } else {  // More complex case: Dragging amplifier waveforms when multiple filters are displayed, arranged by filter.
        // Here we assume that all amplifier waveforms are displayed in the first N sections: 1, 2,..., N,
        // where N = the total number of filters displayed.
        if (index == -1) {  // Before first waveform.
            return (displayList.at(0).sectionID <= numFiltersDisplayed);
        }
        if (index < -1) {   // Drop between waveforms...
            const DisplayedWaveform* wave1 = &displayList[-(index + 2)];
            if (index == -displayList.size() - 1) {  // After last waveform.
                return (wave1->sectionID <= numFiltersDisplayed);
            } else {
                const DisplayedWaveform* wave2 = &displayList[-(index + 1)];
                if (wave1->sectionID <= numFiltersDisplayed || wave2->sectionID <= numFiltersDisplayed) {  // ...but only in amp sections...
                    return true;
                } else {
                    return false;
                }
            }
        } else {            // Drop after selected waveform...
            const DisplayedWaveform* wave1 = &displayList[index];
            if (index == displayList.size() - 1) {  // After last waveform.
                return (wave1->sectionID <= numFiltersDisplayed);
            } else {
                const DisplayedWaveform* wave2 = &displayList[index + 1];
                if (wave1->sectionID <= numFiltersDisplayed || wave2->sectionID <= numFiltersDisplayed) {  // ...but only in amp sections...
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
}

int CDisplayListManager::calculateYCoords(QList<DisplayedWaveform>& list, int y, int labelHeight, double zoomFactor, int& minSpacing)
{
    int halfLabelHeight = ceil((double)labelHeight / 2.0) + 1;
    const int YExtraClickArea = 2;
    bool showDisabledChannels = displayStatus.isShowDisabledChannels();
    int topBottomMargin = labelHeight;

    minSpacing = std::numeric_limits<int>::max();
    for (int i = 0; i < list.size(); ++i) {
        int y1 = y;
        if (showDisabledChannels || list[i].isEnabled()) {
            list[i].yTopLimit = y;
            y += halfLabelHeight + zoomFactor * list[i].spacingAbove;
            list[i].yCoord = y;
            list[i].yTop = (y1 + y) / 2 - YExtraClickArea;
            y += halfLabelHeight + zoomFactor * list[i].spacingBelow;
            list[i].yBottom = (list[i].yCoord + y) / 2 + YExtraClickArea;
            list[i].yBottomLimit = y;
            int ySpacing = y - y1;
            if (ySpacing > 0 && ySpacing < minSpacing) {
                minSpacing = ySpacing;
            }
        } else {
            list[i].yCoord = y;
            list[i].yTopLimit = y;
            list[i].yTop = y;
            list[i].yBottom = y;
            list[i].yBottomLimit = y;
        }
    }
    y += topBottomMargin;
    return y;
}

void CDisplayListManager::addYOffset(QList<DisplayedWaveform>& list, int yOffset)
{
    for (int i = 0; i < list.size(); ++i) {
        list[i].yCoord += yOffset;
        list[i].yTop += yOffset;
        list[i].yBottom += yOffset;
        list[i].yTopLimit += yOffset;
        list[i].yBottomLimit += yOffset;
    }
}

void CDisplayListManager::selectSingleWaveform(WaveIndex waveIndex)
{
    if (waveIndex.index >= 0) {
        deselectAllWaveforms();
        if (waveIndex.inPinned) {
            selectWaveform(pinnedList.at(waveIndex.index).waveName);
        } else {
            if (!displayList.at(waveIndex.index).isDivider()) {
                selectWaveform(displayList.at(waveIndex.index).waveName);
            }
        }
    }
}

bool CDisplayListManager::selectNextOrPreviousWaveform(bool selectNext)
{
    bool pinned = true;
    for (int i = 0; i < displayList.size(); ++i) {
        if (displayList[i].isSelected()) {
            pinned = false;
            break;
        }
    }

    int firstSelected = -1;
    if (selectNext) {
        for (int i = 0; i < numDisplayedWaveforms(pinned); ++i) {
            if (displayedWaveform(i, pinned)->isSelected()) {
                firstSelected = i;
                break;
            }
        }
    } else {
        for (int i = numDisplayedWaveforms(pinned) - 1; i >= 0; --i) {
            if (displayedWaveform(i, pinned)->isSelected()) {
                firstSelected = i;
                break;
            }
        }
    }
    if (firstSelected == -1) return false;

    int newIndex = -1;
    if (selectNext) {
        for (int i = firstSelected + 1; i < numDisplayedWaveforms(pinned); ++i) {
            if (!displayedWaveform(i, pinned)->isSelected() && !displayedWaveform(i, pinned)->isDivider()) {
                newIndex = i;
                break;
            }
        }
    } else {
        for (int i = firstSelected - 1; i >= 0; --i) {
            if (!displayedWaveform(i, pinned)->isSelected() && !displayedWaveform(i, pinned)->isDivider()) {
                newIndex = i;
                break;
            }
        }
    }
    if (newIndex == -1) return false;

    deselectAllWaveforms();
    selectWaveform(displayedWaveform(newIndex, pinned)->waveName);
    return (displayedWaveform(firstSelected, pinned)->isCurrentlyVisible &&
            !displayedWaveform(newIndex, pinned)->isCurrentlyVisible);
}

void CDisplayListManager::selectWaveform(const QString& nativeName, bool select)
{
    emit signalUpdateSelectedWavename(nativeName);
    signalManager.setChannelSelected(nativeName.toStdString(), select);
}

void CDisplayListManager::deselectAllWaveforms()
{
    for (int index = 0; index < signalManager.numChannels(); index++) {
        UiChannel* signalChannel = signalManager.channelByIndex(index);
        signalChannel->setIsSelected(false);
    }
}

void CDisplayListManager::updateOrderInState(int numFiltersDisplayed, bool arrangeByFilter)
{
    int numAmplifierChannels = signalManager.numChannels();

    if (!displayList.isEmpty()) {
        int index = 0;
        if (displayList[0].isAmplifier()) {
            int waveformsInFirstSection = numWaveformsInFirstSection(displayList);
            if (arrangeByFilter) {
                for (int i = 0; i < waveformsInFirstSection; ++i) {
                    UiChannel* channel = displayList.at(i).channel;
                    if (!channel) {
                        std::cerr << "CDisplayListManager::updateOrderInState: Channel not found: " <<
                                displayList.at(i).waveNameWithoutFilter().toStdString() << '\n';
                        return;
                    }
                    if (channel) channel->setUserOrder(i);
                }
            } else {
                int order = 0;
                for (int i = 0; i < waveformsInFirstSection; i += numFiltersDisplayed) {
                    UiChannel* channel = displayList.at(i).channel;
                    if (!channel) {
                        std::cerr << "MultiWaveformPlot::updateOrderInState: Channel not found: " <<
                                displayList.at(i).waveNameWithoutFilter().toStdString() << '\n';
                        return;
                    }
                    if (channel) channel->setUserOrder(order++);
                }
            }
            index += numFiltersDisplayed * (waveformsInFirstSection + 1);  // + 1 to account for section divider
        }

        int numOtherChannelsFound = 0;

        for (int i = index; i < displayList.size(); ++i) {
            WaveformType type = displayList.at(i).waveformType;
            if (type == WaveformDivider) continue;
            UiChannel* channel = signalManager.channelByName(displayList.at(i).waveName);
            if (!channel) {
                std::cerr << "MultiWaveformPlot::updateOrderInState: Channel not found: " <<
                        displayList.at(i).waveName.toStdString() << '\n';
                return;
            }
            channel->setUserOrder(numOtherChannelsFound);
            ++numOtherChannelsFound;
        }
    }
    // Note: Any function calling this function should also call systemState.forceUpdate().
}

std::vector<bool> CDisplayListManager::selectionRecord() const
{
    std::vector<bool> record;

    for (int j = 0; j < signalManager.numChannels(); ++j) {
        UiChannel* channel = signalManager.channelByIndex(j);
        if (channel) {
            record.push_back(channel->isSelected());
        }
    }
    
    return record;
}

bool CDisplayListManager::selectionRecordsAreEqual(const std::vector<bool>& a, const std::vector<bool>& b) const
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < (int) a.size(); ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

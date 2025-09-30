/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_SCROLL_BAR_H
#define __C_SCROLL_BAR_H

#include "nxwaveformwidget_global.h"
#include <vector>
#include <QtWidgets>

BEGIN_NX_NAMESPACE

struct UiScrollBarState
{
    int range;						// Scroll bar operates from 0 to this value.
    std::vector<int> topPosition;	// Current position of scroll bar on each page; always between 0 and (range - pageSize)
    int pageSize;					// Size of one page.  Must be <= range.  Sets scroll bar size.  Increment of movement for page up/down.
    int stepSize;					// Increment of movement from up/down button or cursor up/down
    double zoomFactor;				// Number from 1.0 to ZoomLimit used to expand display
};

class MultiWaveformPlot;
class ScrollBar
{
public:
    ScrollBar(QRect position, MultiWaveformPlot* parent_);

    void resize(QRect position);

    int getTopPosition() const;
    inline int getStepSize() const { return scrollState.stepSize; }
    inline double getZoomFactor() const { return scrollState.zoomFactor; }

    UiScrollBarState getState() const;
    void restoreState(const UiScrollBarState& savedState);
    QString getStateString() const { return scrollBarStateToString(getState()); }
    void restoreStateFromString(const QString& stateString) { restoreState(scrollBarStateFromString(stateString)); }
    void adjustToNewNumberOfPages(int numPages);

    void setRange(int range_);
    void setPageSize(int pageSize_);
    inline void setStepSize(int stepSize_) { scrollState.stepSize = stepSize_; }
    inline void zoomIn() { scrollState.zoomFactor = qMin(scrollState.zoomFactor * ZoomStepMultiple, ZoomLimit); }
    inline void zoomOut() { scrollState.zoomFactor = qMax(scrollState.zoomFactor / ZoomStepMultiple, 1.0); }

    void scroll(int delta);

    // Handle mouse and keypress events, and return true if display needs to be updated.
    bool handleMousePress(QPoint position);
    bool handleMouseRelease();
    bool handleMouseMove(QPoint position);
    bool handleWheelEvent(int delta, bool shiftHeld, bool controlHeld);
    bool handleKeyPressEvent(int key);

    const QColor scrollBarColor = QColor("#3D3D3D");
    const QColor scrollBarActiveColor = Qt::white;

    void draw(QPainter &painter);

private:
    MultiWaveformPlot* parent;

    QImage upArrow;
    QImage upArrowActive;
    QImage downArrow;
    QImage downArrowActive;

    // Long-term state
    UiScrollBarState scrollState;

    // Short-term state
    bool upButtonActive;
    bool downButtonActive;
    bool barActive;
    int barGrabPoint;
    int barGrabPosition;

    // Clickable regions
    QRect scrollFrame;
    QRect scrollBar;
    QRect upButton;
    QRect downButton;
    QRect scrollBarBounds;

    const double ZoomLimit = 50.0;
    const double ZoomStepMultiple = 1.5;

    void setTopPosition(int topPosition_, int page);
    static QString scrollBarStateToString(const UiScrollBarState& state);
    static UiScrollBarState scrollBarStateFromString(const QString& stateString);
};

END_NX_NAMESPACE

#endif

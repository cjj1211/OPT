#include "multiwaveformplot.h"
#include "scrollbar.h"

ScrollBar::ScrollBar(QRect position, MultiWaveformPlot* parent_) :
    parent(parent_),
    upButtonActive(false),
    downButtonActive(false),
    barActive(false),
    barGrabPoint(0),
    barGrabPosition(0)
{
    scrollState.range = 100;
    scrollState.topPosition.resize(1, 0);
    scrollState.pageSize = 10;
    scrollState.stepSize = 1;
    scrollState.zoomFactor = 1.0;

    upArrow = QImage(":/PatientAdmin/images/Recorder/scroll_up_badge.png");
    upArrowActive = QImage(":/PatientAdmin/images/Recorder/scroll_up_active_badge.png");
    downArrow = QImage(":/PatientAdmin/images/Recorder/scroll_down_badge.png");
    downArrowActive = QImage(":/PatientAdmin/images/Recorder/scroll_down_active_badge.png");

    resize(position);
}

void ScrollBar::resize(QRect position)
{
    scrollFrame = position.adjusted(0, position.width() + 4, 0, -position.width() - 5);
    upButton = QRect(position.x(), position.y(), position.width(), position.width());
    downButton = QRect(position.x(), position.bottom() - position.width(), position.width(), position.width());
    scrollBar = scrollFrame.adjusted(2, 2, -1, 1);

    scrollBarBounds = scrollFrame;
    scrollBarBounds = scrollBarBounds.united(upButton);
    scrollBarBounds = scrollBarBounds.united(downButton);
    scrollBarBounds = scrollBarBounds.united(scrollBar);
}

int ScrollBar::getTopPosition() const
{
    return scrollState.topPosition[0]; // TODO 当前不再区分port，index默认为0
}

void ScrollBar::setRange(int range_)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    scrollState.range = range_;
    if (scrollState.pageSize > scrollState.range) {
        scrollState.pageSize = scrollState.range;
    }
    setTopPosition(scrollState.topPosition[pageIndex], pageIndex);
}

void ScrollBar::setTopPosition(int topPosition_, int page)
{
    if (topPosition_ < 0) {
        scrollState.topPosition[page] = 0;
    } else if (topPosition_ > scrollState.range - scrollState.pageSize) {
        scrollState.topPosition[page] = scrollState.range - scrollState.pageSize;
    } else {
        scrollState.topPosition[page] = topPosition_;
    }
}

void ScrollBar::setPageSize(int pageSize_)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    if (pageSize_ < 1) {
        scrollState.pageSize = 1;
    } else if (pageSize_ <= scrollState.range) {
        scrollState.pageSize = pageSize_;
    } else {
        scrollState.pageSize = scrollState.range;
    }
    setTopPosition(scrollState.topPosition[pageIndex], pageIndex);

}

void ScrollBar::scroll(int delta)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    setTopPosition(scrollState.topPosition[pageIndex] + delta, pageIndex);
}

UiScrollBarState ScrollBar::getState() const
{
    UiScrollBarState savedState;
    savedState.range = scrollState.range;
    savedState.topPosition.resize(scrollState.topPosition.size());
    for (int i = 0; i < (int) savedState.topPosition.size(); ++i) {
        savedState.topPosition[i] = scrollState.topPosition[i];
    }
    savedState.pageSize = scrollState.pageSize;
    savedState.stepSize = scrollState.stepSize;
    savedState.zoomFactor = scrollState.zoomFactor;
    return savedState;
}

void ScrollBar::restoreState(const UiScrollBarState& savedState)
{
    scrollState.range = scrollState.range;
    for (int i = 0; i < (int) savedState.topPosition.size(); ++i) {
        if (i < (int) scrollState.topPosition.size()) {
            scrollState.topPosition[i] = savedState.topPosition[i];
        }
    }
    scrollState.pageSize = savedState.pageSize;
    scrollState.stepSize = savedState.stepSize;
    scrollState.zoomFactor = savedState.zoomFactor;
}

// Format: Numbers separated by commas in the following order:
// "range,pageSize,stepSize,zoomFactor,topPosition[0],topPosition[1],topPosition[2],..."
QString ScrollBar::scrollBarStateToString(const UiScrollBarState& state)
{
    QString stateString;
    const QString Separator = ",";
    stateString += QString::number(state.range);
    stateString += Separator;
    stateString += QString::number(state.pageSize);
    stateString += Separator;
    stateString += QString::number(state.stepSize);
    stateString += Separator;
    stateString += QString::number(state.zoomFactor);
    stateString += Separator;
    for (int i = 0; i < (int) state.topPosition.size(); ++i) {
        stateString += QString::number(state.topPosition[i]);
        if (i < (int) state.topPosition.size() - 1) {
            stateString += Separator;
        }
    }
    return stateString;
}

UiScrollBarState ScrollBar::scrollBarStateFromString(const QString& stateString)
{
    UiScrollBarState state;
    const QString Separator = ",";
    int numPages = stateString.count(Separator) - 3;
    state.topPosition.resize(numPages);
    state.range = stateString.section(Separator, 0, 0).toInt();
    state.pageSize = stateString.section(Separator, 1, 1).toInt();
    state.stepSize = stateString.section(Separator, 2, 2).toInt();
    state.zoomFactor = stateString.section(Separator, 3, 3).toDouble();
    for (int i = 0; i < numPages; ++i) {
        state.topPosition[i] = stateString.section(Separator, i + 4, i + 4).toInt();
    }
    return state;
}

void ScrollBar::adjustToNewNumberOfPages(int numPages)
{
    scrollState.topPosition.clear();
    scrollState.topPosition.resize(numPages, 0);
}

bool ScrollBar::handleMousePress(QPoint position)
{
    if (!scrollBarBounds.contains(position)) return false;
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0

    int topPositionOld = scrollState.topPosition[pageIndex];
    if (scrollFrame.contains(position)) {
        if (scrollBar.contains(position)) {
            barActive = true;
            barGrabPoint = position.y();
            barGrabPosition = scrollState.topPosition[pageIndex];
        } else if (position.y() < scrollBar.top()) {
            scroll(-scrollState.pageSize);
        } else if (position.y() > scrollBar.bottom()) {
            scroll(scrollState.pageSize);
        }
    } else if (upButton.contains(position)) {
        upButtonActive = true;
        scroll(-scrollState.stepSize);
    } else if (downButton.contains(position)) {
        downButtonActive = true;
        scroll(scrollState.stepSize);
    }
    return (scrollState.topPosition[pageIndex] != topPositionOld);
}

bool ScrollBar::handleMouseRelease()
{
    bool barActiveOld = barActive;
    bool upButtonActiveOld = upButtonActive;
    bool downButtonActiveOld = downButtonActive;
    if (barActive) {
        barActive = false;
    } else {
        upButtonActive = false;
        downButtonActive = false;
    }
    return (barActive != barActiveOld) ||
            (upButtonActive != upButtonActiveOld) ||
            (downButtonActive != downButtonActiveOld);
}

bool ScrollBar::handleMouseMove(QPoint position)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0

    if(pageIndex < 0)
    {
        return false;
    }

    int topPositionOld = scrollState.topPosition[pageIndex];
    if (barActive) {
        double deltaY = position.y() - barGrabPoint;
        setTopPosition(barGrabPosition + scrollState.range * (deltaY / (double)scrollFrame.height()), pageIndex);
    } else if (upButtonActive) {
        if (!upButton.contains(position)) {
            upButtonActive = false;
        }
    } else if (downButtonActive) {
        if (!downButton.contains(position)) {
            downButtonActive = false;
        }
    }
    return (scrollState.topPosition[pageIndex] != topPositionOld);
}

bool ScrollBar::handleWheelEvent(int delta, bool shiftHeld, bool controlHeld)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    int topPositionOld = scrollState.topPosition[pageIndex];
    double zoomFactorOld = scrollState.zoomFactor;
    if (!shiftHeld && !controlHeld) {
        if (delta > 0) scroll(-ceil((double)scrollState.pageSize / 8.0));
        else if (delta < 0) scroll(ceil((double)scrollState.pageSize / 8.0));
    } else if (controlHeld & !shiftHeld) {
        if (delta > 0) zoomIn();
        else if (delta < 0) zoomOut();
    }
    return (scrollState.topPosition[pageIndex] != topPositionOld || scrollState.zoomFactor != zoomFactorOld);
}

bool ScrollBar::handleKeyPressEvent(int key)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    int topPositionOld = scrollState.topPosition[pageIndex];
    int pageSizeOld = scrollState.pageSize;

    switch (key) {
    case Qt::Key_PageUp:
        scroll(-scrollState.pageSize);
        break;
    case Qt::Key_PageDown:
        scroll(scrollState.pageSize);
        break;
    case Qt::Key_Home:
        setTopPosition(0, pageIndex);
        break;
    case Qt::Key_End:
        setTopPosition(scrollState.range - scrollState.pageSize, pageIndex);
        break;
    }
    return (scrollState.topPosition[pageIndex] != topPositionOld || scrollState.pageSize != pageSizeOld);
}

void ScrollBar::draw(QPainter &painter)
{
    int pageIndex = 0;// TODO 当前不再区分port，index默认为0
    painter.setPen(scrollBarColor);

    if (upButtonActive) {
        painter.drawImage(upButton.topLeft(), upArrowActive);
    } else {
        painter.drawImage(upButton.topLeft(), upArrow);
    }

    if (downButtonActive) {
        painter.drawImage(downButton.topLeft(), downArrowActive);
    } else {
        painter.drawImage(downButton.topLeft(), downArrow);
    }

    
    painter.setPen(scrollBarColor);
    painter.setBrush(Qt::NoBrush);
    //painter.drawRect(scrollFrame);

    scrollBar = scrollFrame.adjusted(2, 2, -1, -1);
    scrollBar.moveTop(scrollBar.top() +
                      ((double)scrollState.topPosition[pageIndex] / (double)scrollState.range) * (scrollBar.height() + 1));
    scrollBar.setHeight(((double)scrollState.pageSize / (double)scrollState.range) * scrollBar.height());
    if (scrollState.topPosition[pageIndex] == scrollState.range - scrollState.pageSize) {
        scrollBar.moveTop(scrollFrame.bottom() - scrollBar.height());
    }
    if (scrollBar.width() % 2 == 1)
    {
        scrollBar.setWidth(scrollBar.width() + 1);
    }

    QPainterPath path;
    path.addRoundedRect(QRectF(scrollBar.left(), scrollBar.top() - scrollBar.width() / 2, scrollBar.width(), scrollBar.width()), scrollBar.width() / 2, scrollBar.width() / 2);
    painter.fillPath(path, barActive ? scrollBarActiveColor : scrollBarColor);
    painter.drawPath(path);

    path.addRoundedRect(QRectF(scrollBar.left(), scrollBar.bottom() - scrollBar.width() / 2, scrollBar.width(), scrollBar.width()), scrollBar.width() / 2, scrollBar.width() / 2);
    painter.fillPath(path, barActive ? scrollBarActiveColor : scrollBarColor);
    painter.drawPath(path);
    
    painter.fillRect(scrollBar, barActive ? scrollBarActiveColor : scrollBarColor);
}

#include <limits>
#include "waveformdisplaycolumn.h"
#include "multiwaveformplot.h"
#include "DisplayedWaveform.h"
#include <QtMath>

BEGIN_NX_NAMESPACE

MultiWaveformPlot::MultiWaveformPlot(int columnIndex_,  WaveformDisplayManager* waveformManager_,
	                                     WaveformDisplayColumn* parent_) :
    QWidget(parent_),
    columnIndex(columnIndex_),
    waveformManager(waveformManager_),
    parent(parent_)
{
    LOG("Beginning of MultiWaveformPlot ctor");
    calculatePixelsPerMM();
    setBackgroundRole(QPalette::Window);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    LOG_INFO(std::format("About to set minimum size. x size: {},  ... y size: {} ", MINIMUM_X_SIZE_MULTIWAVEFORM_PLOT, MINIMUM_Y_SIZE_MULTIWAVEFORM_PLOT));
    setMinimumSize(MINIMUM_X_SIZE_MULTIWAVEFORM_PLOT, MINIMUM_Y_SIZE_MULTIWAVEFORM_PLOT);
    LOG("Completed setMinimumSize()");
    setMouseTracking(true);  // If we don't do this, mouseMoveEvent() is active only when mouse button is pressed.
    setFocusPolicy(Qt::StrongFocus);
    QApplication::focusWidget();

    showPinned = true;

    hoverWaveIndex.index = -1;
    hoverWaveIndex.inPinned = false;

    dragState.dragging = false;
    dragState.fromPinned = false;

    mouseButtonDown = false;
    outOfBoundsClick = false;

    numFiltersDisplayed = 0;
    arrangeByFilter = false;

    LOG("About to create DisplayListManager");
    listManager = new CDisplayListManager(displayList, pinnedList, this);
    connect(listManager, SIGNAL(signalUpdateSelectedWavename(QString)), SIGNAL(signalUpdateSelectedWavename(QString)));
    LOG("Created DisplayListManager");

    int fontSize = 8;
    labelFont = new QFont("Courier", fontSize);
    labelFontMetrics = new QFontMetrics(*labelFont);
    labelHeight = labelFontMetrics->ascent() + 1;

    labelWidthIndexOld = -1;
    
    
    labelWidth = labelFontMetrics->horizontalAdvance(QString::fromLocal8Bit("XXA-008 100kΩ")) + 12;

    LOG("About to call calculateScreenRegions()");
    calculateScreenRegions();
    LOG("Completed calculateScreenRegions(). About to call updateFromState()");
    updateFromState();
    LOG("Completed updateFromState(). About to create ScrollBar");

    scrollBar = new ScrollBar(regionScrollBar,  this);
    LOG("Created ScrollBar");

    saveBadge = QImage(":/PatientAdmin/images/Recorder/save_badge.png");
    tcpBadge = QImage(":/PatientAdmin/images/Recorder/tcp_badge.png");
    saveSelectedBadge = QImage(":/PatientAdmin/images/Recorder/save_badge_selected.png");
    tcpSelectedBadge = QImage(":/PatientAdmin/images/Recorder/tcp_badge_selected.png");
    unpinBadge = QImage(":/PatientAdmin/images/Recorder/unpin_badge_angle.png");
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    LOG("Created various QImages");

    rec.difference_X = 0;
    rec.difference_Y = 0;
    rec.draw_shap = DRAW_RECT;

    rec.framePen = QPen(QColor(207, 207, 207,188), 1);
    rec.redPointPen = QPen(QColor(181, 181, 181,188), 2);
    rec.is_mouse_pressed = false;
    rec.timerID = startTimer(20);
    rectInitRegion();
    /// 开启鼠标实时追踪
    setMouseTracking(true);

    connect(this, SIGNAL(signalUpdateDisplayParam()), parent, SIGNAL(signalUpdateDisplayParam()));
    connect(this, SIGNAL(signalDispalyWaveNamesChanged()), parent, SIGNAL(signalDispalyWaveNamesChanged()));
    LOG("End of MultiWaveformPlot ctor");
}

MultiWaveformPlot::~MultiWaveformPlot()
{
    delete scrollBar;
    delete labelFontMetrics;
    delete labelFont;
    killTimer(rec.timerID);
}

void MultiWaveformPlot::calculateScreenRegions()
{
    const int xSize = width();
    const int ySize = height();

    const int xScrollBarLeft = xSize - 18;
    const int yWaveformTop = labelHeight + 10;
    const int yWaveformBottom = ySize - 8;

    regionWaveforms = QRect(labelWidth, yWaveformTop, xScrollBarLeft - labelWidth - 1,
        yWaveformBottom - yWaveformTop);
    regionRuler = QRect(labelWidth, yWaveformTop + 15, xScrollBarLeft - labelWidth - 21,
        yWaveformBottom - yWaveformTop - 15);
	regionLabels = QRect(0, yWaveformTop, labelWidth, yWaveformBottom - yWaveformTop);
	regionTimeAxis = QRect(labelWidth, 0, xScrollBarLeft - labelWidth - 1, yWaveformTop + 3);
    regionAboveLabels = QRect(0, 0, xSize, yWaveformTop);
    regionBelowLabels = QRect(0, yWaveformBottom, xSize, ySize - yWaveformBottom);
    regionScrollBar = QRect(xScrollBarLeft, yWaveformTop + 4, 14, yWaveformBottom - yWaveformTop - 5);
    regionUnpinSymbols = QRect(xScrollBarLeft, yWaveformTop, 15, 1);

    if (rec.rect_left < regionRuler.left()) {
        rec.rect_left = regionRuler.left() + 1;
    }
    if (rec.rect_left + rec.rect_width > regionRuler.right()) {
        rec.rect_width = regionRuler.right() - rec.rect_left - 1;
    }
    if (rec.rect_top < regionRuler.top()) {
        rec.rect_top = regionRuler.top() + 1;
    }
    if (rec.rect_top + rec.rect_height > regionRuler.bottom()) {
        rec.rect_height = regionRuler.bottom() - rec.rect_top - 1;
    }
    if (rec.rect_height < 0) {
        rec.rect_height = 1;
    }
    if (rec.rect_width < 0) {
        rec.rect_width = 1;
    }
    rectNewUpdateRegion();

    parent->setWaveformWidth(regionWaveforms.width());
}

QSize MultiWaveformPlot::minimumSizeHint() const
{
    return {MINIMUM_X_SIZE_MULTIWAVEFORM_PLOT, MINIMUM_Y_SIZE_MULTIWAVEFORM_PLOT};
}

QSize MultiWaveformPlot::sizeHint() const
{
    return {MINIMUM_X_SIZE_MULTIWAVEFORM_PLOT, MINIMUM_Y_SIZE_MULTIWAVEFORM_PLOT};
}

void MultiWaveformPlot::updateFromState()
{
    LOG("Beginning of updateFromState()");
    parent->setWaveformWidth(regionWaveforms.width());

    const QStringList displayWaveforms = signalManager.getDisplayListAmplifiers();

    LOG("About to populate display list");
    listManager->populateDisplayList(displayWaveforms);
    LOG("Populated display list");

    numFiltersDisplayed = 1; 
    arrangeByFilter = false; // (recorderService->arrangeBy->getValue() == "Filter");
    
    //parent->updateVisibleWaveName();
    update();
    LOG("End of updateFromState()");
}

void MultiWaveformPlot::setDrawShap(DRAW_SHAP_E d)
{
    rec.draw_shap = d;
}

void MultiWaveformPlot::drawPoint(QPainter& painter)
{
    painter.save();
    switch (rec.draw_shap) {
    case (DRAW_RECT): {

        const auto physicalSize = qApp->primaryScreen()->physicalSize(); // 屏幕物理尺寸 单位 mm
        const auto size = qApp->primaryScreen()->size();
        const auto pixelsPerYm = size.height() * 10.0 / physicalSize.height(); // 每mm宽度对应的像素数
        auto recWidth = (rec.rect_width / pixelsPerMM) / displayStatus.getPaperSpeed();
        auto freq = 1.0 / recWidth;
        const auto yScaleFactor = waveformManager->getYScaleFactor(WidebandYScale);
        auto recHeigt = rec.rect_height/yScaleFactor/10.0/*(rec.rect_height / pixelsPerYm) * recorderService->yScaleWide->getNumericValue()*/;
        if (recWidth < 0)
        {
            recWidth = -recWidth;
        }
        if (recHeigt < 0)
        {
            recHeigt = -recHeigt;
        }
        painter.setPen(rec.framePen);//绘制边框线
        QBrush brush;
        brush.setStyle(Qt::SolidPattern);
        brush.setColor(QColor(181, 181, 181, 10));
        painter.setBrush(brush);

        painter.drawRect(QRect(rec.rect_left, rec.rect_top, rec.rect_width, rec.rect_height));

        painter.setPen(rec.redPointPen);//绘制八个点

        painter.drawPoints(rec.rect_polygon);

        int recHeigtNum = (rec.rect_height / pixelsPerYm);

        int recWidthNum = (rec.rect_width / pixelsPerMM * 10.0);
        if (recHeigtNum > 0)
        {
            auto y = rec.rect_low_left_y;
            int num = 1;
            while (y > rec.rect_low_left_y - rec.rect_height)
            {
               
                
                y = y - 0.2 * pixelsPerYm;
                if (num%5==0)
                {
                    painter.drawLine(QPoint(rec.rect_left, y), QPoint(rec.rect_left + 3, y));
                }
                else
                {
                    painter.drawLine(QPoint(rec.rect_left, y), QPoint(rec.rect_left + 1, y));
                }
                num++;
            }

        }
        if (recWidthNum > 0)
        {
            auto x = rec.rect_top_left_x;
            int num = 1;
            while (x < rec.rect_top_left_x + rec.rect_width)
            {
             
                x = x + 0.2 * pixelsPerMM * 10.0;
                if (num%5==0)
                {
                    painter.drawLine(QPoint(x, rec.rect_low_left_y - 3), QPoint(x, rec.rect_low_left_y));
                }
                else
                {
                    painter.drawLine(QPoint(x, rec.rect_low_left_y - 1), QPoint(x, rec.rect_low_left_y));
                }
                num++;
            }
        }

        QFont font;
        font.setWeight(55);
        font.setFamily("Microsoft YaHei");
        font.setPixelSize(15);
        painter.setFont(font);

        painter.setPen(QPen(QColor(181, 181, 181, 255), 2));
        painter.drawText(QPoint(rec.rect_left - 2, rec.rect_top - 2), QString::number(recHeigt * 10, 'f', 2) + QString::fromLocal8Bit(" μv"));
        painter.drawText(QPoint(rec.rect_low_right_x + 2, rec.rect_low_right_y + 2), QString::number(recWidth, 'f', 3) + " s/" + QString::number(freq, 'f', 3) + " Hz");
    }break;

  
    }
    painter.restore();
}   

bool MultiWaveformPlot::event(QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {  // Implement custom tool tips.
	    const QHelpEvent* helpEvent = dynamic_cast<QHelpEvent*>(event);

	    const QPoint cursor = mapFromGlobal(QCursor::pos());
        if (regionLabels.contains(cursor)) {
            calculateDisplayYCoords();
            hoverWaveIndex = findSelectedWaveform(cursor.y());
            if (hoverWaveIndex.index >= 0) {
	            UiChannel* channel = listManager->displayedWaveform(hoverWaveIndex)->channel;
                auto toolTip = channel->getCustomName();
                if (channel->getCustomName() != channel->getNativeName()) toolTip += " (" + channel->getNativeName() + ")";
                if (channel->getSignalType() == UiSignalType::AmplifierSignal) {
                    if (channel->isImpedanceValided()) {
                        toolTip += "\n" + tr("Z = ") + channel->getImpedanceMagnitudeString() + " " +
                                channel->getImpedancePhaseString();
                    }
                }
                
                toolTip += QString::fromLocal8Bit(" 点击空格键可以隐藏选中的通道");
                QToolTip::showText(helpEvent->globalPos(), toolTip);
            } else {
                QToolTip::hideText();
            }
        } else {
            QToolTip::hideText();
        }
        return true;
    }
    return QWidget::event(event);
}

void MultiWaveformPlot::paintEvent(QPaintEvent* /* event */)
{
    //QElapsedTimer timer;
    //timer.restart();

    QPainter painter(&image);        //用来绘制图像

    bool showDisabledChannels = displayStatus.isShowDisabledChannels();
    bool clipWaveforms = displayStatus.isClipWaveforms();

    // Clear old display.
    QRect imageFrame(rect());
    const QColor BackgroundColor = QColor(displayStatus.getBackgroundColor());
    painter.fillRect(imageFrame, QBrush(BackgroundColor));
    
    calculateDisplayYCoords();
    QPoint cursor = mapFromGlobal(QCursor::pos());
    hoverWaveIndex = findSelectedWaveform(cursor.y());
    bool cursorInWaveformArea = regionLabels.contains(cursor) || regionWaveforms.contains(cursor);

    // Draw vertical time lines.
    if (regionTimeAxis.contains(cursor)) {
        painter.setClipRegion(imageFrame);
        drawCursorTimeLine(painter, regionTimeAxis.left(), cursor.x());
    }

    QRect plotClipRegion = regionWaveforms.united(regionLabels).united(regionScrollBar);
    painter.setClipRegion(plotClipRegion);
    int clipX = plotClipRegion.left();
    int clipWidth = plotClipRegion.width();

    int yPosition;
   
    // Draw y = 0 baseline.
    if (hoverWaveIndex.index >= 0 && cursorInWaveformArea && !dragState.dragging) {
        yPosition = listManager->displayedWaveform(hoverWaveIndex)->yCoord;
        painter.setPen(QColor(72, 72, 72));
        painter.drawLine(QPoint(regionWaveforms.left(), yPosition), QPoint(regionWaveforms.right() - 1, yPosition));
        auto a = regionWaveforms.left();
        auto b = regionWaveforms.right();
    }
    
    if (showPinned) {
        // Draw pinned/unpinned waveform divider.
        if (!pinnedList.isEmpty()) {
            waveformManager->drawDivider(painter, pinnedYDivider, regionLabels.left() + 2, regionWaveforms.right());
        }

        // Draw pinned waveforms.
        for (int i = 0; i < pinnedList.size(); ++i) {
            yPosition = pinnedList.at(i).yCoord;
            // If dragging, spread out waveforms around potential drop spot.
            if (dragState.dragging && dragState.fromPinned && hoverWaveIndex.inPinned) {
                yPosition = spreadAroundDraggingTarget(yPosition, hoverWaveIndex.index, i);
            }
            QString waveName = pinnedList.at(i).waveName;
            bool enabled = pinnedList.at(i).isEnabled();
            if (enabled || showDisabledChannels) {
                if (enabled) {
                    bool hoverHighlight = hoverWaveIndex.inPinned && i == hoverWaveIndex.index && cursorInWaveformArea;
                    QColor waveColor =  adjustedColor(pinnedList.at(i), hoverHighlight);
                    if (clipWaveforms) {
                        QRect individualClipRegion = QRect(clipX, pinnedList.at(i).yTopLimit, clipWidth,
                                                           pinnedList.at(i).yBottomLimit - pinnedList.at(i).yTopLimit);
                        painter.setClipRegion(individualClipRegion);
                    }
                    waveformManager->draw(painter, waveName, QPoint(regionWaveforms.left(), yPosition), waveColor);
                    if (clipWaveforms) {
                        painter.setClipRegion(plotClipRegion);
                    }
                }
                QColor waveColor = adjustedColor(pinnedList.at(i));
                UiChannel* channel = pinnedList.at(i).channel;
                drawWaveformLabel(painter, waveName, channel, QPoint(regionLabels.right(), yPosition), waveColor,
                                  pinnedList.at(i).isSelected() ? Qt::black : Qt::white);
                painter.drawImage(QPoint(regionScrollBar.left() + 1, yPosition - 5), unpinBadge);
                pinnedList[i].isCurrentlyVisible = enabled;
            } else {
                pinnedList[i].isCurrentlyVisible = false;
            }
        }
    }

    // Draw waveforms.
    bool isVisbleWavesChanged = false;
    for (int index = 0; index < displayList.size(); ++index) {
        yPosition = displayList.at(index).yCoord;
        // If dragging, spread out waveforms around potential drop spot.
        if (dragState.dragging && !dragState.fromPinned && !hoverWaveIndex.inPinned &&
                listManager->isValidDragTarget(hoverWaveIndex, numFiltersDisplayed, arrangeByFilter)) {
            yPosition = spreadAroundDraggingTarget(yPosition, hoverWaveIndex.index, index);
        }
        if (yPosition >= pinnedYDivider + YExtra && yPosition <= regionWaveforms.bottom() - YExtra) {
            bool enabled = displayList.at(index).isEnabled();
            if (!displayList.at(index).isDivider()) {
                if (enabled || showDisabledChannels) {
                    QString waveName = displayList.at(index).waveName;
                    if (enabled) {
                        bool hoverHighlight = hoverWaveIndex.inPinned && index == hoverWaveIndex.index && cursorInWaveformArea;
                        QColor waveColor = adjustedColor(displayList.at(index), hoverHighlight);
                        if (clipWaveforms) {
                            QRect individualClipRegion = QRect(clipX, displayList.at(index).yTopLimit, clipWidth,
                                                               displayList.at(index).yBottomLimit - displayList.at(index).yTopLimit);
                            painter.setClipRegion(individualClipRegion);
                        }
                        waveformManager->draw(painter, waveName, QPoint(regionWaveforms.left(), yPosition), waveColor);
                        if (clipWaveforms) {
                            painter.setClipRegion(plotClipRegion);
                        }
                    }
                    QColor waveColor = adjustedColor(displayList.at(index));
                    UiChannel* channel = displayList.at(index).channel;
                    drawWaveformLabel(painter, waveName, channel, QPoint(regionLabels.right(), yPosition), waveColor,
                                      displayList.at(index).isSelected() ? Qt::black : Qt::white);
                }
            } else {
                waveformManager->drawDivider(painter, yPosition, regionLabels.left() + 2, regionWaveforms.right());
            }
            if (displayList[index].isCurrentlyVisible != enabled)
            {
                isVisbleWavesChanged = true;
                displayList[index].isCurrentlyVisible = enabled;
            }
            
        } else {
            if (displayList[index].isCurrentlyVisible )
            {
                isVisbleWavesChanged = true;
                displayList[index].isCurrentlyVisible = false;
            }
        }
    }

    if (isVisbleWavesChanged) {
        emit signalDispalyWaveNamesChanged();
    }

    // Draw 'drag target' marker between waveforms.
    drawBetweenWaveformMarker(painter, regionLabels.right());

    // Draw vertical scale bar.
    if (hoverWaveIndex.index >= 0 && !dragState.dragging) {
	    const DisplayedWaveform* wave = listManager->displayedWaveform(hoverWaveIndex);
        if (wave->isEnabled()) {
	        int maxHeight;
	        int yTop = hoverWaveIndex.inPinned ? regionWaveforms.top() : pinnedYDivider;
            if (hoverWaveIndex.index > 0) {
                const DisplayedWaveform *prevWave = listManager->displayedWaveform(hoverWaveIndex.index - 1, hoverWaveIndex.inPinned);
                maxHeight = static_cast<int>(0.67 * static_cast<double>(wave->yCoord - prevWave->yCoord));
            } else {
                const DisplayedWaveform *topWave = listManager->displayedWaveform(0, hoverWaveIndex.inPinned);
                maxHeight = static_cast<int>(0.67 * static_cast<double>(topWave->yCoord - yTop));
            }
            maxHeight = qMax(maxHeight, 12);
        }
    }

    // Draw vertical scan line. 绘制竖向时间线，时间线显示当前记录位置
    if ((!pinnedList.isEmpty() && showPinned) || !displayList.isEmpty()) {
        int x = displayStatus.getValidDataIndex();
        painter.setPen(QColor(97, 69, 157));
        painter.drawLine(QLineF(x + regionWaveforms.left(), regionWaveforms.top(),
            x + regionWaveforms.left(), regionWaveforms.bottom()));
    }

    painter.setClipRegion(imageFrame);

    // Draw time axis and scroll bar.
    drawTimeAxis(painter, regionTimeAxis.bottomLeft());
    scrollBar->draw(painter);

    // Draw dragged waveform labels.
    if (dragState.dragging) {
        int yPos;
        for (int index = 0; index < listManager->numDisplayedWaveforms(dragState.fromPinned); ++index) {
	        if (DisplayedWaveform* wave = listManager->displayedWaveform(index, dragState.fromPinned); wave->isSelected()) {
                yPos = wave->yCoord + dragDelta.y();
                if (yPos > -labelHeight && yPos < rect().bottom() + labelHeight) {
                    QColor waveColor = adjustedColor(*wave, true);
                    drawWaveformLabel(painter, wave->waveName, wave->channel,
                                      QPoint(regionLabels.right() + dragDelta.x(), yPos), waveColor, Qt::black);
                }
            }
        }
    }
    if (displayStatus.isShowRuler() == true)
    {
        drawPoint(painter);
        drawYScaleRuler(painter, imageFrame.x() + imageFrame.width() - 250, imageFrame.y() + imageFrame.height() - 80);
        drawXTimeRuler(painter, imageFrame.x() + imageFrame.width() - 250, imageFrame.y() + imageFrame.height() - 80);
    }


    QStylePainter stylePainter(this);
    stylePainter.drawImage(0, 0, image);
    
    //qDebug() << "plot time (ms): " << timer.nsecsElapsed() / 1.0e6 << ", Current paperSpeed: " << QString::fromStdString(recorderService->tPaperSpeed->getDisplayValueString());
}

QColor MultiWaveformPlot::adjustedColor(const DisplayedWaveform& waveform, bool hoverSelect) const
{
    const bool enabled = waveform.isEnabled();
    const bool selected = waveform.isSelected() || hoverSelect;

    QColor color;
    //if(systemConfig.getOscilloscopeConfig().enableWaveformColor)
    //{
        color = waveform.getColor();
    //}
    //else
    //{
    //    color = QColor(systemConfig.getOscilloscopeConfig().waveformColor);
    //}
    if (enabled && !selected) return color;
    if (!enabled) color = DisabledColor;
    if (selected) color = color.lighter(170);
    return color;
}

int MultiWaveformPlot::spreadAroundDraggingTarget(int y, int hoverIndex, int index) const
{
    int yPosition = y;
    int n;
    if (hoverIndex < 0) n = -hoverIndex - 2;
    else n = hoverIndex;
    if (n == index + 1) yPosition = y - 2;
    if (n == index    ) yPosition = y - 4;
    if (n == index - 1) yPosition = y + 4;
    if (n == index - 2) yPosition = y + 2;
    return yPosition;
}

void MultiWaveformPlot::rectInitRegion()
{
    rec.rect_left = labelWidth + 10;
    rec.rect_top = 40;
    rec.rect_width = 101;
    rec.rect_height = 101;
    rec.rect_mouse_pos = RECT_OUTSIDE;
    rectNewUpdateRegion();
}

void MultiWaveformPlot::rectNewUpdateRegion()
{
    rec.rect_top_left_x = rec.rect_left;            
    rec.rect_top_left_y = rec.rect_top;
    rec.rect_top_right_x = rec.rect_left + rec.rect_width; 
    rec.rect_top_right_y = rec.rect_top;
    rec.rect_low_left_x = rec.rect_left;            
    rec.rect_low_left_y = rec.rect_top + rec.rect_height;
    rec.rect_low_right_x = rec.rect_left + rec.rect_width; 
    rec.rect_low_right_y = rec.rect_top + rec.rect_height;

    rec.rect_polygon.clear();
}

void MultiWaveformPlot::rectUpdateRegion()
{
    switch (rec.rect_mouse_pos) {
    case (RECT_UPPER): 
        if (rec.newMousePos.y() >= rec.rect_top + rec.rect_height) break;
        rec.rect_top += rec.difference_Y; 
        rec.rect_height -= rec.difference_Y; break;//上边界
    case (RECT_LOWER): 
        if (rec.newMousePos.y() <= rec.rect_top) break;
        rec.rect_height += rec.difference_Y; break; //下边界
    case (RECT_LEFT): 
        if (rec.newMousePos.x() >= rec.rect_left + rec.rect_width) break;
        rec.rect_left += rec.difference_X; 
        rec.rect_width -= rec.difference_X; break;//左边界
    case (RECT_RIGHT): 
        if (rec.newMousePos.x() <= rec.rect_left) break;
        rec.rect_width += rec.difference_X; break; //右边界
    case (RECT_LEFTUPPER): {//左上角
        if (rec.newMousePos.y() < rec.rect_top + rec.rect_height) {
            rec.rect_top += rec.difference_Y;
            rec.rect_height -= rec.difference_Y;
        }
        if (rec.newMousePos.x() < rec.rect_left + rec.rect_width) {
            rec.rect_left += rec.difference_X;
            rec.rect_width -= rec.difference_X;
        }
    }break;
    case (RECT_LEFTLOWER): {//左下角
        if (rec.newMousePos.y() > rec.rect_top) {
            rec.rect_height += rec.difference_Y;
        }
        if (rec.newMousePos.x() < rec.rect_left + rec.rect_width) {
            rec.rect_left += rec.difference_X;
            rec.rect_width -= rec.difference_X;
        }
    }break;
    case (RECT_RIGHTLOWER): {//右下角
        if (rec.newMousePos.y() > rec.rect_top) {
            rec.rect_height += rec.difference_Y;
        }
        if (rec.newMousePos.x() > rec.rect_left) {
            rec.rect_width += rec.difference_X;
        }
    }break;
    case (RECT_RIGHTUPPER): {//右上角
        if (rec.newMousePos.y() < rec.rect_top + rec.rect_height) {
            rec.rect_top += rec.difference_Y;
            rec.rect_height -= rec.difference_Y;
        }
        if (rec.newMousePos.x() > rec.rect_left) {
            rec.rect_width += rec.difference_X;
        }
    }break;
    case (RECT_INSIDE): {//内部
        if (rec.rect_top + rec.difference_Y > regionRuler.top() &&
            rec.rect_top + rec.difference_Y + rec.rect_height < regionRuler.bottom())
        {
            rec.rect_top += rec.difference_Y;
        }
        if (rec.rect_left + rec.difference_X > regionRuler.left() &&
            rec.rect_left + rec.difference_X + rec.rect_width < regionRuler.right())
        {
            rec.rect_left += rec.difference_X;
        }
        
    }break;
    case (RECT_OUTSIDE): return;//外部
    }
    rectNewUpdateRegion();
}

RECT_MOUSE_POSITION_E MultiWaveformPlot::rectGetMousePos(int pos_x, int pos_y)
{
    if (pos_x < rec.rect_top_left_x || pos_x > rec.rect_top_right_x || pos_y < rec.rect_top_left_y || pos_y > rec.rect_low_left_y) {
        this->setCursor(QCursor(Qt::ArrowCursor));
        return RECT_OUTSIDE;
    }
    else if (pos_y <= rec.rect_top_left_y + rec.BoundaryRange) { //1:左上角  2:右上角  3:上边缘
        if (pos_x <= rec.rect_top_left_x + rec.BoundaryRange) { this->setCursor(QCursor(Qt::SizeFDiagCursor)); return RECT_LEFTUPPER; }
        else if (pos_x >= rec.rect_top_right_x - rec.BoundaryRange) { this->setCursor(QCursor(Qt::SizeBDiagCursor)); return RECT_RIGHTUPPER; }
        else { this->setCursor(QCursor(Qt::SizeVerCursor));  return RECT_UPPER; }
    }
    else if (pos_y >= rec.rect_low_left_y - rec.BoundaryRange) { //1:左下角  2:右下角  3:下边缘
        if (pos_x <= rec.rect_low_left_x + rec.BoundaryRange) { this->setCursor(QCursor(Qt::SizeBDiagCursor)); return RECT_LEFTLOWER; }
        else if (pos_x >= rec.rect_low_right_x - rec.BoundaryRange) { this->setCursor(QCursor(Qt::SizeFDiagCursor)); return RECT_RIGHTLOWER; }
        else { this->setCursor(QCursor(Qt::SizeVerCursor));  return RECT_LOWER; }
    }
    else if (pos_x <= rec.rect_top_left_x + rec.BoundaryRange) {   //左边缘
        this->setCursor(QCursor(Qt::SizeHorCursor));  return RECT_LEFT;
    }
    else if (pos_x >= rec.rect_top_right_x - rec.BoundaryRange) {  //右边缘
        this->setCursor(QCursor(Qt::SizeHorCursor));  return RECT_RIGHT;
    }
    else {
        this->setCursor(QCursor(Qt::SizeAllCursor));
        return RECT_INSIDE;
    }
}

void MultiWaveformPlot::calculateDisplayYCoords()
{
	const int topBottomMargin = labelHeight;
	const double zoomFactor = scrollBar->getZoomFactor();
    int minSpacing;
    pinnedYDivider = regionWaveforms.top();

    int y = topBottomMargin;

    if (!pinnedList.isEmpty() && showPinned) {
        y = listManager->calculateYCoords(pinnedList, y, labelHeight, zoomFactor, minSpacing);
        listManager->addYOffset(pinnedList, regionWaveforms.top());
        pinnedYDivider += y;
        y += topBottomMargin;
    }

    regionScrollBar.setTop(pinnedYDivider);
    regionUnpinSymbols.setBottom(pinnedYDivider);
    scrollBar->resize(regionScrollBar);

    y = listManager->calculateYCoords(displayList, y, labelHeight, zoomFactor, minSpacing);
    listManager->addYOffset(displayList, regionWaveforms.top() - scrollBar->getTopPosition());

    scrollBar->setRange(y);
    scrollBar->setPageSize(regionWaveforms.height());
    scrollBar->setStepSize(minSpacing);
}

void MultiWaveformPlot::calculatePixelsPerMM()
{
	const auto physicalSize = qApp->primaryScreen()->physicalSize(); // unit mm
	const auto size = qApp->primaryScreen()->size();
    pixelsPerMM = size.width() * 1.0 / physicalSize.width();
}

WaveIndex MultiWaveformPlot::findSelectedWaveform(int y) const
{
    WaveIndex w{};
    if (y < pinnedYDivider) {
        w = { listManager->findSelectedWaveform(pinnedList, y), true };
    } else {
        w = { listManager->findSelectedWaveform(displayList, y), false };
    }
    return w;
}

void MultiWaveformPlot::mouseMoveEvent(QMouseEvent* event)
{
    if (event->pos() == lastMousePos) return;  // Only deal with actual mouse moves.  (Apparently mouseMoveEvent is also
    else lastMousePos = event->pos();          // triggered by mouse button presses, which we handle separately.)
    rec.newMousePos = event->pos();
    if (displayStatus.isShowRuler() == true 
        && event->pos().x() > regionRuler.left()
        && event->pos().x() < regionRuler.right()
        && event->pos().y() > regionRuler.top() + 20
        && event->pos().y() < regionRuler.bottom())
    {
        rec.newMousePos = event->pos();
      
        const auto physicalSize = qApp->primaryScreen()->physicalSize(); // unit mm
        const auto size = qApp->primaryScreen()->size();
        pixelsPerMM = size.width() * 1.0 / physicalSize.width();

            if (rec.is_mouse_pressed) {
                
                rec.difference_X = rec.newMousePos.x() - rec.oldMousePos.x();
                rec.difference_Y = rec.newMousePos.y() - rec.oldMousePos.y();
               
                switch (rec.draw_shap) {
                case (DRAW_RECT):rectUpdateRegion(); break;

                case (DRAW_NO):break;
                }
            }
            else {
                switch (rec.draw_shap) {
                case (DRAW_RECT):rec.rect_mouse_pos = rectGetMousePos(rec.newMousePos.x(), rec.newMousePos.y()); break;

                case (DRAW_NO):break;
                }

            }

            rec.oldMousePos = rec.newMousePos;
       
    }
    

    hoverWaveIndex = findSelectedWaveform(event->pos().y());

    scrollBar->handleMouseMove(event->pos());

    if (dragState.dragging && !dragState.fromPinned) {
        // Scroll up or down if pointer is above or below labels.
        if (regionBelowLabels.contains(event->pos())) {
            scrollBar->scroll(scrollBar->getStepSize());
        } else if (regionAboveLabels.contains(event->pos()) || hoverWaveIndex.inPinned) {
            scrollBar->scroll(-scrollBar->getStepSize());
        }
    } else if (!dragState.dragging && event->buttons() & Qt::LeftButton) {
        if (regionLabels.contains(event->pos()) && !outOfBoundsClick) {
            // Start dragging.
            const WaveIndex clickedWaveform = findSelectedWaveform(event->pos().y());
            dragState.dragging = true;
            dragState.fromPinned = clickedWaveform.inPinned;
            if (clickedWaveform.index >= 0) {
                if (!(listManager->displayedWaveform(clickedWaveform)->isSelected())) {
                    if (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)) {
                        listManager->selectSingleWaveform(clickedWaveform);
                        //signalManager.forceUpdateSystemState();
                    }
                } else {
                    dragState.dragging = false;
                }
            }
        }
    }
    if (dragState.dragging) {
        dragDelta = event->pos() - clickPoint;
        if (!dragState.fromPinned) {
            dragDelta += QPoint(0, scrollBar->getTopPosition());
        }
    }
    update();
}

void MultiWaveformPlot::mousePressEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) {  // Ignore center and right button releases.
        QWidget::mousePressEvent(event);
        return;
    }
    if (displayStatus.isShowRuler() == true)

    {
        rec.is_mouse_pressed = true;
    }
   
    bool needToUpdate = false;
    bool needToUpdateState = false;
    mouseButtonDown = true;
    clickPoint = event->pos();
    if (event->pos().y() >= pinnedYDivider) {
        clickPoint += QPoint(0, scrollBar->getTopPosition());
    }

    // Handle mouse clicks on scroll bar.
    if (scrollBar->handleMousePress(event->pos())) needToUpdate = true;

    // Handle mouse clicks on waveforms.
    //if (regionLabels.contains(event->pos()) ||
    //        regionWaveforms.contains(event->pos())) {
	   // const WaveIndex clickedWaveform = findSelectedWaveform(event->pos().y());
    //    if (clickedWaveform.index >= 0) {
	   //     const DisplayedWaveform* clickedWave = listManager->displayedWaveform(clickedWaveform);
    //        if (!(event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier)) {
    //            // Control-click to select or deselect multiple waveforms.
    //            if (!clickedWaveform.inPinned) {  // Don't allow multiple waveform selection in pinned list; too many problems.
    //                listManager->selectNonAdjacentWaveforms(clickedWaveform, numFiltersDisplayed, arrangeByFilter);
    //                needToUpdateState = true;
    //            }
    //        } else if ((event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)) {
    //            // Shift-click to select multiple adjacent waveforms.
    //            if (!clickedWaveform.inPinned) {  // Don't allow multiple waveform selection in pinned list; too many problems.
    //                if (arrangeByFilter && numFiltersDisplayed > 1 && clickedWave->sectionID <= numFiltersDisplayed &&
    //                        listManager->sectionIDOfSelectedWaveforms() <= numFiltersDisplayed &&
    //                        listManager->selectedWaveformsAreAmplifiers()) {
    //                    // Add new selections if all are amp waveforms of different filters.
    //                    const int waveformsPerSection = listManager->numWaveformsInFirstSection(displayList);
    //                    listManager->selectAdjacentWaveforms(clickedWaveform, waveformsPerSection);
    //                    needToUpdateState = true;

    //                } else if (clickedWave->sectionID == listManager->sectionIDOfSelectedWaveforms()) {
    //                    // Add new selections if all are same type as old.
    //                    listManager->selectAdjacentWaveforms(clickedWaveform);
    //                    needToUpdateState = true;
    //                }
    //            }
    //        }

    //        if (clickedWave->isDivider()) outOfBoundsClick = true;
    //    }
    //} else {
    //    outOfBoundsClick = true;
    //}

    if (needToUpdateState) {
        //signalManager.forceUpdateSystemState();
    } else if (needToUpdate) {
        update();
    }
}

void MultiWaveformPlot::mouseReleaseEvent(QMouseEvent* event)
{
    bool needToUpdate = false;
    bool needToUpdateState = false;
    mouseButtonDown = false;
    outOfBoundsClick = false;
    const QPoint dropPoint = event->pos();
    if (displayStatus.isShowRuler() == true)
    {
        rec.is_mouse_pressed = false;
    }
 
    // Release scroll bar, if held.
    if (scrollBar->handleMouseRelease()) needToUpdate = true;

    // Move waveforms, if selected.
    if (dragState.dragging) {
        dragState.dragging = false;
        if (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)) {
	        const WaveIndex dropPosition = findSelectedWaveform(dropPoint.y());
            if (listManager->isValidDragTarget(dropPosition, numFiltersDisplayed, arrangeByFilter)) {
                bool realMove = false;
                if (dragState.fromPinned && dropPosition.inPinned) {
                    // Move pinned waveforms.
                    if (dropPosition.index < -1) {  // Drop between waveforms.
                        realMove = listManager->moveSelectedWaveforms(pinnedList, -(dropPosition.index + 2));
                    } else {                        // Drop after selected waveform.
                        realMove = listManager->moveSelectedWaveforms(pinnedList, dropPosition.index);
                    }
                } else if (!dragState.fromPinned && !dropPosition.inPinned) {
                    // Move unpinned waveforms.
                    if (arrangeByFilter && numFiltersDisplayed > 1 && listManager->selectedWaveformsAreAmplifiers()) {
                        if (dropPosition.index < -1) {  // Drop between waveforms.
                            realMove = listManager->moveSelectedWaveformsArrangedByFilter(displayList,
                                                                                                   -(dropPosition.index + 2),
                                                                                                   numFiltersDisplayed);
                        } else {                        // Drop after selected waveform.
                            realMove = listManager->moveSelectedWaveformsArrangedByFilter(displayList,
                                                                                                   dropPosition.index,
                                                                                                   numFiltersDisplayed);
                        }
                    } else {
                        if (dropPosition.index < -1) {  // Drop between waveforms.
                            realMove = listManager->moveSelectedWaveforms(displayList, -(dropPosition.index + 2));
                        } else {                        // Drop after selected waveform.
                            realMove = listManager->moveSelectedWaveforms(displayList, dropPosition.index);
                        }
                    }
                }
                if (realMove) {
                    needToUpdateState = true;
                } else {
                    needToUpdate = true;
                }
            }
            if (needToUpdateState) {
                listManager->updateOrderInState(numFiltersDisplayed, arrangeByFilter);
            }
        }
    } else if (regionLabels.contains(dropPoint) ||
               regionWaveforms.contains(dropPoint)) {
        if (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)) {
            // Click and release to select a single waveform.
            const WaveIndex clickedWaveform = findSelectedWaveform(dropPoint.y());

            const std::vector<bool> before = listManager->selectionRecord();
            listManager->selectSingleWaveform(clickedWaveform);
            const std::vector<bool> after = listManager->selectionRecord();
            //needToUpdateState = !listManager->selectionRecordsAreEqual(before, after);
        }
    } else if (regionUnpinSymbols.contains(dropPoint)) {
        if (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)) {
            // Click and release on unpin symbol to remove a single pinned waveform.
            const WaveIndex clickedWaveform = findSelectedWaveform(dropPoint.y());
            if (clickedWaveform.inPinned) {
                pinnedList.removeAt(clickedWaveform.index);
                needToUpdate = true;
            }
        }
    }

    if (needToUpdateState) {
        //signalManager.forceUpdateSystemState();
    } else if (needToUpdate) {
        update();
    }
}

void MultiWaveformPlot::wheelEvent(QWheelEvent* event)
{
    bool needToUpdate = false;
    const int topPositionOld = scrollBar->getTopPosition();
    const bool shiftHeld = event->modifiers() & Qt::ShiftModifier;
    const bool controlHeld = event->modifiers() & Qt::ControlModifier;
    const int delta = event->angleDelta().y();

    if (scrollBar->handleWheelEvent(delta, shiftHeld, controlHeld)) needToUpdate = true;

    if (shiftHeld && !controlHeld) {
        if (delta > 0) {
            if (displayStatus.getPaperSpeedIndex() > 0) {
                displayStatus.shiftPaperSpeed(-1);
                //signalManager.forceUpdateSystemState();
            }
        } else if (delta < 0) {
            if (displayStatus.getPaperSpeedIndex() < static_cast<int>(displayStatus.getPaperSpeedNumberOfItems()) - 1) {
                displayStatus.shiftPaperSpeed(1);
                //signalManager.forceUpdateSystemState();
            }
        }
        waveformManager->setTPaperSpeed();
        emit signalUpdateDisplayParam();
    }

    if (dragState.dragging && !dragState.fromPinned) {
        dragDelta.setY(dragDelta.y() - (topPositionOld - scrollBar->getTopPosition()));
        needToUpdate = true;
    }
    if (needToUpdate) update();
}

void MultiWaveformPlot::keyPressEvent(QKeyEvent* event)
{
    bool needToUpdate = false;
    bool needToUpdateState = false;
    const bool ctrlPressed = event->modifiers() & Qt::ControlModifier;

    const int topPositionOld = scrollBar->getTopPosition();

    if (scrollBar->handleKeyPressEvent(event->key())) {
        needToUpdate = true;
    } else if (event->matches(QKeySequence::MoveToNextPage)) {  // Check for alternate (Mac) key sequences.
        if (scrollBar->handleKeyPressEvent(Qt::Key_PageDown)) needToUpdate = true;
    } else if (event->matches(QKeySequence::MoveToPreviousPage)) {
        if (scrollBar->handleKeyPressEvent(Qt::Key_PageUp)) needToUpdate = true;
    } else if (event->matches(QKeySequence::MoveToStartOfLine)) {
        if (scrollBar->handleKeyPressEvent(Qt::Key_Home)) needToUpdate = true;
    } else if (event->matches(QKeySequence::MoveToEndOfLine)) {
        if (scrollBar->handleKeyPressEvent(Qt::Key_End)) needToUpdate = true;
    } else if (event->key() == Qt::Key_Down || event->matches(QKeySequence::MoveToNextLine)) {
        needToUpdate = true;
        needToUpdateState = true;
        if (listManager->selectNextWaveform()) scrollBar->handleKeyPressEvent(Qt::Key_PageDown);
    } else if (event->key() == Qt::Key_Up || event->matches(QKeySequence::MoveToPreviousLine)) {
        needToUpdate = true;
        needToUpdateState = true;
        if (listManager->selectPreviousWaveform()) scrollBar->handleKeyPressEvent(Qt::Key_PageUp);
    }
       
    if (!(event->isAutoRepeat())) { // Ignore additional 'keypresses' from auto-repeat if key is held down.
        switch (event->key()) {

        case Qt::Key_Space:  // Space bar: Toggle waveform enable.
            listManager->toggleSelectedWaveforms();
            needToUpdateState = true;
            emit signalToggleSelectedWaveforms();
            break;

        case Qt::Key_P:
            if (ctrlPressed) {  // Ctrl+P: Pin selected waveforms.
	            const int numSelected = listManager->numSelectedWaveforms();
                if (numSelected > 0 && numSelected <= 12) {  // Limit max number to prevent accidentally pinning everything.
                    listManager->pinSelectedWaveforms();
                    needToUpdate = true;
                }
            }
            break;

        case Qt::Key_U:
            if (ctrlPressed) {  // Ctrl+U: Unpin selected waveforms.
                listManager->unpinSelectedWaveforms();
                needToUpdate = true;
            }
            break;

        default:
            QWidget::keyPressEvent(event);
        }
    }

    if (dragState.dragging) {
        dragDelta.setY(dragDelta.y() - (topPositionOld - scrollBar->getTopPosition()));
        needToUpdate = true;
    }
    if (needToUpdateState) {
        //displayStatus.forceUpdateSystemState();
    } else if (needToUpdate) {
        update();
    }
}

void MultiWaveformPlot::resizeEvent(QResizeEvent*) {
    calculateScreenRegions();
    scrollBar->resize(regionScrollBar);
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    update();
}

void MultiWaveformPlot::timerEvent(QTimerEvent*)
{
    update();
}

// 绘制时间轴纵向分割线
void MultiWaveformPlot::drawVerticalTimeLines(QPainter &painter, int xPosition) const
{
	const auto yTop = static_cast<qreal>(regionWaveforms.top());
	const auto yBottom = static_cast<qreal>(regionWaveforms.bottom());
	const auto oldColor = painter.pen().color();
    painter.setPen(displayStatus.getTimelineBackground());
	const auto x = static_cast<float>(xPosition);
    painter.drawLine(QPointF(x, yTop), QPointF(x, yBottom));
    painter.setPen(oldColor);
}

void MultiWaveformPlot::drawCursorTimeLine(QPainter& painter, int xPosition, int xCursor) const
{
    const float xSize = static_cast<float>(displayStatus.getDispWaveAreaLength());
    const auto yTop = static_cast<qreal>(regionWaveforms.top());
    const auto yBottom = static_cast<qreal>(regionWaveforms.bottom());
    if (xCursor >= xPosition && xCursor <= xPosition + xSize) {
        painter.setPen(QColor(72, 72, 72));
        painter.drawLine(QPoint(xCursor, yTop), QPoint(xCursor, yBottom));
    }
}


int MultiWaveformPlot::timeAxisStep() const
{
    int step = pixelsPerMM * 10 * displayStatus.getPaperSpeed();
    return step;
}

// 绘制时间轴
void MultiWaveformPlot::drawTimeAxis(QPainter& painter, QPoint position) const
{
    QColor penColor;
    if (const auto backgroundColor = QColor(displayStatus.getBackgroundColor()); backgroundColor.value() < 128) {  // Ensure time axis is visible against background.
        penColor = Qt::white;
    }
    else {
        penColor = Qt::black;
    }
    painter.setPen(penColor);
    auto dispWidth = displayStatus.getDispWaveAreaLength();
    auto timestamps = displayStatus.getTimeAxisLabels();

    const auto y = static_cast<float>(position.y());
    auto prePos = -2000;
    for (auto pos : timestamps.keys())
    {
        auto timeStr = timestamps[pos];
        const int pixelsWide = labelFontMetrics->horizontalAdvance(timeStr);
        if (pos - prePos > pixelsWide)
        {
            const int xShift = pixelsWide / 2;
            painter.drawText(position.x() + pos - xShift, y - labelHeight, timeStr);
            prePos = pos;
        }
        drawVerticalTimeLines(painter, position.x() + pos);
    }
    drawVerticalTimeLines(painter, position.x());
    drawVerticalTimeLines(painter, position.x() + displayStatus.getDispWaveAreaLength());

    painter.drawLine(QPointF(position.x(), regionWaveforms.top()), QPointF(position.x() + dispWidth, regionWaveforms.top()));
}

void MultiWaveformPlot::drawBetweenWaveformMarker(QPainter &painter, const int xPosition) const
{
    if (!dragState.dragging) return;
    if (dragState.fromPinned && !hoverWaveIndex.inPinned) return;
    if (!dragState.fromPinned && hoverWaveIndex.inPinned) return;
    if (!(listManager->isValidDragTarget(hoverWaveIndex, numFiltersDisplayed, arrangeByFilter))) return;

    int y;
    int ySpace;
    const int length = listManager->numDisplayedWaveforms(hoverWaveIndex.inPinned);
    if (hoverWaveIndex.index == -1) {
        y = listManager->displayedWaveform(0, hoverWaveIndex.inPinned)->yTop - 5;
    } else if (hoverWaveIndex.index == -(length + 1) || hoverWaveIndex.index == length - 1) {
        y = listManager->displayedWaveform(length - 1, hoverWaveIndex.inPinned)->yBottom + 6;
    } else if (hoverWaveIndex.index >= 0) {
        const DisplayedWaveform* wave1 = listManager->displayedWaveform(hoverWaveIndex);
        const DisplayedWaveform* wave2 = listManager->displayedWaveform(hoverWaveIndex.index + 1, hoverWaveIndex.inPinned);
        y = (wave1->yBottom + wave2->yTop) / 2;
        ySpace = wave2->yTop - wave1->yBottom;
    } else {
        const DisplayedWaveform* wave1 = listManager->displayedWaveform(-hoverWaveIndex.index - 2, hoverWaveIndex.inPinned);
        const DisplayedWaveform* wave2 = listManager->displayedWaveform(-hoverWaveIndex.index - 1, hoverWaveIndex.inPinned);
        y = (wave1->yBottom + wave2->yTop) / 2;
        ySpace = wave2->yTop - wave1->yBottom;
    }
    ySpace = qBound(18, ySpace, labelHeight + 8);
    painter.setPen(Qt::white);
    painter.drawRect(QRect(xPosition - labelWidth + 5, y - ySpace / 2 + 5,
                     labelWidth - 7, ySpace - 8));
}

// 绘制波形标签
void MultiWaveformPlot::drawWaveformLabel(QPainter &painter, const QString& name, const UiChannel* channel, QPoint position,
                                          QColor color, QColor textColor) const
{

    QString mainText;
    const bool isAmpSignal = channel->getSignalType() == AmplifierSignal;

    if (!isAmpSignal) {
        mainText = channel->getNativeName();
    }
	else
    {
        mainText = channel->getCustomName();
    }

    if (!channel->isImpedanceValided()) {
        mainText = tr("n/a");
    }
    else {
        mainText = channel->getImpedanceMagnitudeString();
    }

    QString channelName = channel->getCustomName();
    if (isAmpSignal && channelName.length() > 8) {
        channelName = EllipsisSymbol + channelName.right(7);
    }
    mainText = channelName + " " + mainText;

    painter.setFont(*labelFont);

	const int halfHeight = (labelHeight - 1) / 2;
	const int y1 = position.y() - halfHeight;
	const int y2 = position.y() + halfHeight - 1;

	const int x = position.x() + 3;
	const int x1 = x - labelWidth + 2;
    painter.fillRect(x1, y1, labelWidth - 6, labelHeight, color);
    const int xText = x1 + 5;
    painter.setPen(textColor);
    painter.drawText(xText, y2, mainText);
}

// 绘制幅值标尺
void MultiWaveformPlot::drawYScaleRuler(QPainter& painter, const int xPosition, const int yPosition) const
{
	const auto yScaleValue = displayStatus.getYScale() * 10;
	QString label ;

    if(yScaleValue >= 1000000)
    {
        label = QString::number(yScaleValue / 1000000.0, 'f', 2) + "v";
    }
    else if(yScaleValue >= 1000)
    {
        label = QString::number(yScaleValue / 1000.0, 'f', 2) + "mv";
    }
    else
    {
        label = QString::number(yScaleValue, 'f', 2) + "uv";
    }


    const auto yScaleFactor = waveformManager->getYScaleFactor(WidebandYScale);
    const int height = static_cast<int>(round(yScaleFactor * yScaleValue));
    constexpr int halfWidth = 2;

    painter.fillRect(xPosition - 1, yPosition - height - 1, 3, height + 2, Qt::black);
    painter.fillRect(xPosition - halfWidth - 1, yPosition - 1, 2 * halfWidth + 3, 3, Qt::black);
    painter.fillRect(xPosition - halfWidth - 1, yPosition - height - 1, 2 * halfWidth + 3, 3, Qt::black);
    painter.setPen(Qt::white);
    painter.drawLine(QPoint(xPosition, yPosition), QPoint(xPosition, yPosition - height));
    painter.drawLine(QPoint(xPosition - halfWidth, yPosition), QPoint(xPosition + halfWidth, yPosition));
    painter.drawLine(QPoint(xPosition - halfWidth, yPosition - height), QPoint(xPosition + halfWidth, yPosition - height));
    constexpr int xTextMax = 200;
    const int yText = yPosition - height;
    const QRect boundingBox(xPosition - xTextMax, yText, xTextMax - 5, labelHeight + 2);
    painter.setPen(Qt::black);
    painter.drawText(boundingBox.adjusted(-1, -1, -1, -1), Qt::AlignRight, label);
    painter.drawText(boundingBox.adjusted(1, -1, 1, -1), Qt::AlignRight, label);
    painter.drawText(boundingBox.adjusted(-1, 1, -1, 1), Qt::AlignRight, label);
    painter.drawText(boundingBox.adjusted(1, 1, 1, 1), Qt::AlignRight, label);
    painter.setPen(Qt::white);
    painter.drawText(boundingBox, Qt::AlignRight, label);

}

// 绘制水平时间轴比例尺
void MultiWaveformPlot::drawXTimeRuler(QPainter& painter, const int xPosition, const int yPosition) const
{
    const int width = pixelsPerMM*10;
	const double tScaleValue = 10.0 / displayStatus.getPaperSpeed() ;
    const QString label = QString::number(tScaleValue, 'f', 3) + "s";
    const auto yScaleFactor = waveformManager->getYScaleFactor(WidebandYScale);
    constexpr int halfHeight = 2;
    painter.fillRect(xPosition - 1, yPosition  - 1, width + 2,  3, Qt::black); // ruler background 
    painter.fillRect(xPosition - 1, yPosition - halfHeight - 1, 3, 2 * halfHeight + 3, Qt::black);
    painter.fillRect(xPosition + width - 1, yPosition - halfHeight - 1, 3, 2 * halfHeight + 3, Qt::black);
    painter.setPen(Qt::white);
    painter.drawLine(QPoint(xPosition, yPosition), QPoint(xPosition + width, yPosition ));
    painter.drawLine(QPoint(xPosition, yPosition - halfHeight), QPoint(xPosition , yPosition + halfHeight));
    painter.drawLine(QPoint(xPosition + width , yPosition - halfHeight), QPoint(xPosition + width, yPosition  + halfHeight));
    
    constexpr int xTextMax = 200;
    const QRect boundingBox(xPosition + width - 15 , yPosition + 5, xTextMax - 5, labelHeight + 2);

    painter.setPen(Qt::white);
    painter.drawText(boundingBox, Qt::AlignLeft, label);
}

int MultiWaveformPlot::getYScaleHeightAndText(const DisplayedWaveform* waveform, 
                                              int maxHeight, QString& label)
{
	const float yScaleFactor =  waveformManager->getYScaleFactor(waveform->waveName);

    double yValue = static_cast<double>(maxHeight) / yScaleFactor;
    int height = round(displayStatus.getYScale(0) * yScaleFactor);

    label = QString::number(static_cast<float>(height) / yScaleFactor, 'f', 2) + "uv";// QString::fromStdString(yScaleList->getDisplayValueString(0));

    // Adjust scale bar height to largest 'round number' below maxHeight.

    for (int i = displayStatus.getYScalerNumOfItems() - 1; i >= 0; --i) {
        if (displayStatus.getYScale(i) <= yValue) {
            yValue = displayStatus.getYScale(i);
            height = round(yValue * yScaleFactor);
            label = QString::number(height / yScaleFactor, 'f', 2) + "uv";//QString::fromStdString(yScaleList->getDisplayValueString(i));
            break;
        }
    }
    return height;
}

void MultiWaveformPlot::loadWaveformData( UiDisplayDto& uiDisplayDto)
{
    for (const auto& waveform : pinnedList)
    {
        if (waveform.isCurrentlyVisible) {
    
            waveformManager->loadNewData(uiDisplayDto, waveform.waveName);
        }
    }
    constexpr bool loadAllFilters = true;

    for (const auto& waveform : displayList)
    {
        if (waveform.isCurrentlyVisible && !waveform.isDivider()) {
            waveformManager->loadNewData(uiDisplayDto, waveform.waveName);
        }
    }

    // Note: repaint() seems to give slightly smoother animation than update(), but may cause "QWidget::repaint.
    // Recursive repaint detected" crash when columns are added.
    // repaint();

    update();
}


QStringList MultiWaveformPlot::getPinnedWaveNames() const
{
    QStringList pinnedWaveNames;
    for (int i = 0; i < pinnedList.size(); ++i) {
        pinnedWaveNames.append(pinnedList.at(i).waveName);
    }
    return pinnedWaveNames;
}

QStringList MultiWaveformPlot::getRenderWaveNames()
{
    QStringList visibleWaves;
    for(const auto& item: pinnedList)
    {
        if(item.isCurrentlyVisible)
        {
            visibleWaves.append(item.waveName);
        }
    }

    for(const auto& item : displayList)
    {
        if(item.isCurrentlyVisible)
        {
            visibleWaves.append(item.waveName);
        }
    }

    return visibleWaves;
}

void MultiWaveformPlot::setPinnedWaveforms(const QStringList& pinnedWaveNames)
{
    pinnedList.clear();
    for (int i = 0; i < pinnedWaveNames.size(); ++i) {
        listManager->addPinnedWaveform(pinnedWaveNames.at(i));
    }
}
END_NX_NAMESPACE
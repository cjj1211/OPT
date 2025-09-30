/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_MULTI_WAVEFORM_PLOT_H
#define __C_MULTI_WAVEFORM_PLOT_H

#define MINIMUM_X_SIZE_MULTIWAVEFORM_PLOT 350
#define MINIMUM_Y_SIZE_MULTIWAVEFORM_PLOT 400
#include "nxwaveformwidget_global.h"
#include "waveformdisplaymanager.h"
#include "CDisplayListmanager.h"
#include "displayedwaveform.h"
#include "ScrollBar.h"
#include <QtWidgets>
#include <set>
#include <QKeyEvent>
#include <QPoint>
#include <QPen>
#include <QColor>



BEGIN_NX_NAMESPACE 
class WaveformDisplayColumn;
class UiDisplayDto;
const QColor DisabledColor = QColor(80, 80, 80);

struct DragStateData {
    bool dragging;
    bool fromPinned;
};
typedef enum draw_shap_e {
    DRAW_RECT,    //画矩形
    DRAW_ELLIPSE, //画椭圆
    DRAW_NO       //不画
}DRAW_SHAP_E;
/* 用来表示鼠标在矩形区域的位置信息
 *
 */
typedef enum rect_mouse_position_e {
    RECT_UPPER = 0,     //上边缘
    RECT_LOWER = 1,     //下边缘
    RECT_LEFT,        //左边缘
    RECT_RIGHT,       //右边缘
    RECT_LEFTUPPER,   //左上角
    RECT_LEFTLOWER,   //左下角
    RECT_RIGHTLOWER,  //右下角
    RECT_RIGHTUPPER,  //右上角
    RECT_INSIDE,      //区域内部
    RECT_OUTSIDE      //区域外部
}RECT_MOUSE_POSITION_E;
struct RectStruct
{
    bool is_mouse_pressed;//是否按下鼠标
    DRAW_SHAP_E draw_shap;
    QPoint                           newMousePos;
    QPoint                           oldMousePos;
    int                              difference_X;
    int                              difference_Y;
   
    QPen                             framePen;       //用来绘制区域边框
    QPen                             redPointPen;   //用来绘制红色点
    const int                        BoundaryRange = 6;//用来表示边界的宽度范围，用于拖拽
    /* 矩形区域相关变量和函数
     * 注意:QPoint的0点是左上角，横向向右为X的正方向，竖向向下为Y的正方向
     */
    int        rect_left;        //表示矩形右上角的X坐标
    int        rect_top;         //表示矩形右上角的Y坐标
    int        rect_width;       //表示矩形的宽(即水平长度)
    int        rect_height;      //表示矩形的高(即垂直长度)
    int        rect_top_left_x, rect_top_left_y;       //左上
    int        rect_top_right_x, rect_top_right_y;      //右上
    int        rect_low_left_x, rect_low_left_y;       //左下
    int        rect_low_right_x, rect_low_right_y;      //右下
    QPolygon                      rect_polygon;          //装载8个红点的坐标
    RECT_MOUSE_POSITION_E         rect_mouse_pos;
    int                 timerID;


};

class MultiWaveformPlot : public QWidget
{
    Q_OBJECT
public:
    explicit MultiWaveformPlot(int columnIndex_, WaveformDisplayManager* waveformManager_,
                               WaveformDisplayColumn* parent_);
    ~MultiWaveformPlot();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void loadWaveformData(UiDisplayDto& uiDisplayDto);
    inline void updateNow() { update(); }

    void enableSelectedWaveforms(bool enable) { listManager->enableSelectedWaveforms(enable); }

    inline void setIndex(int columnIndex_) { columnIndex = columnIndex_; }
    inline void showPinnedWaveforms(bool enable) { showPinned = enable; update(); }

    QStringList getPinnedWaveNames() const;
    QStringList getRenderWaveNames(); // 获取后端需要渲染的通道名称列表

    void setPinnedWaveforms(const QStringList& pinnedWaveNames);
    void unpinAllWaveforms() { listManager->unpinAllWaveforms(); }

    void adjustToNewNumberOfPorts(int numPorts) { scrollBar->adjustToNewNumberOfPages(numPorts); }
    void setDrawShap(DRAW_SHAP_E d);
    void drawPoint(QPainter& painter);
public slots:
    void updateFromState();
    void addPinnedWaveform(const QString& waveName) { listManager->addPinnedWaveform(waveName); }
    void increaseSpacing() { scrollBar->zoomIn(); update(); }
    void decreaseSpacing() { scrollBar->zoomOut(); update(); }

signals:
    void signalUpdateDisplayParam();
    void signalDispalyWaveNamesChanged();
    void signalToggleSelectedWaveforms();
    void signalUpdateSelectedWavename(QString nativeName);
protected:
    bool event(QEvent* event) override;  // Used for implementing custom tool tips.
//    void contextMenuEvent(QContextMenuEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void timerEvent(QTimerEvent*) override;

private:
    MultiWaveformPlot(const MultiWaveformPlot&) = delete;  // Copying not allowed.
    MultiWaveformPlot& operator=(const MultiWaveformPlot&) = delete;  // Copying not allowed.
    int columnIndex;
    WaveformDisplayManager* waveformManager;
    WaveformDisplayColumn* parent;

    ScrollBar* scrollBar;

    QImage image;

    QImage saveBadge;
    QImage tcpBadge;
    QImage saveSelectedBadge;
    QImage tcpSelectedBadge;

    QImage unpinBadge;

    QList<DisplayedWaveform> displayList;
    QList<DisplayedWaveform> pinnedList;
    int pinnedYDivider;
    WaveIndex hoverWaveIndex;
    bool showPinned;
    CDisplayListManager* listManager;

    int numFiltersDisplayed;
    bool arrangeByFilter;

    // GUI short-term state
    DragStateData dragState;
    bool mouseButtonDown;
    bool outOfBoundsClick;
    QPoint lastMousePos;
    QPoint clickPoint;
    QPoint dragDelta;

    // Label text parameters
    QFont* labelFont;
    QFontMetrics* labelFontMetrics;
    int labelHeight;
    int labelWidth;
    int labelWidthIndexOld;

    // Screen regions used for plotting and mouse calculations
    QRect regionWaveforms;
    QRect regionRuler;
    QRect regionLabels;
    QRect regionTimeAxis;
    QRect regionAboveLabels;
    QRect regionBelowLabels;
    QRect regionScrollBar;
    QRect regionUnpinSymbols;

    static const int YExtra = 5;

    double pixelsPerMM; // 水平方向，每厘米包含的像素数

    int timeAxisStep() const;
    void drawVerticalTimeLines(QPainter &painter, int xPosition) const;
    void drawCursorTimeLine(QPainter& painter, int xPosition, int xCursor = -1) const;

	// 绘制时间轴
    void drawTimeAxis(QPainter &painter, QPoint position) const;
    void drawBetweenWaveformMarker(QPainter &painter, int xPosition) const;
    void drawWaveformLabel(QPainter &painter, const QString& name, const UiChannel* channel, QPoint position, QColor color,
                           QColor textColor) const;
    // 绘制灵敏度纵轴比例尺
	void drawYScaleRuler(QPainter& painter,const int xPosition, const int yPosition) const;
    // 绘制水平比例尺
    void drawXTimeRuler(QPainter& painter, const int xPosition, const int yPosition) const;

    int getYScaleHeightAndText(const DisplayedWaveform* waveform, int maxHeight,
                               QString& label);

    void calculateScreenRegions();
    void calculateDisplayYCoords();
    void calculatePixelsPerMM();

    WaveIndex findSelectedWaveform(int y) const;

    QColor adjustedColor(const DisplayedWaveform& waveform, bool hoverSelect = false) const;
    int spreadAroundDraggingTarget(int y, int hoverIndex, int index) const;
    void                          rectInitRegion();
    void                          rectUpdateRegion();
    void                          rectNewUpdateRegion();
    RECT_MOUSE_POSITION_E         rectGetMousePos(int pos_x, int pos_y);

    RectStruct rec;
};

END_NX_NAMESPACE
#endif

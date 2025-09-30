/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __C_COLOR_GRADIENT_WIDGET_H
#define __C_COLOR_GRADIENT_WIDGET_H
#include "Global.h"
#include <QWidget>
#include <QMap>

BEGIN_NX_NAMESPACE

enum GradientType {
    GT_IMPEDANCE,
    GT_HEAT
};

/** 渐变色组件，可以通过按钮调整颜色区间.
 */
class CColorGradientWidget : public QWidget
{
    Q_OBJECT

public:
    CColorGradientWidget(GradientType gradientType, const QString& gradientTitle, QWidget *parent = nullptr);
    ~CColorGradientWidget();

    QColor getColor(const double value) const;

signals:
    void signalGradientChanged();

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void draw(QPainter& painter, QRect& imageFrame);
    void drawRectWithBorder(QPainter& painter, const QRectF& frame, const QColor& color,const double borderRadius);
    void drawTitle(QPainter& painter);
    void drawImpedanceTags(QPainter& painter);
    void drawHeatTags(QPainter& painter);
    QLinearGradient setupGradient(const QRectF& frame);

private:
    GradientType type;
    QImage image;
    QRect scopeFrame;
    QRectF leftBtnFrame;
    QRectF rightBtnFrame;
    QColor startColor;
    QColor endColor;
    QColor leftBtnColor;
    QColor rightBtnColor;
    QColor backgroundColor;
    QColor borderColor;
    QString title;
    bool isLeftBtnPressed;
    bool isRightBtnPressed;
    double leftBtnPos;
    double rightBtnPos;
    double btnWidth;
    double btnHeight;
};

END_NX_NAMESPACE
#endif
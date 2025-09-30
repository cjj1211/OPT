// /******************************************************************************
// *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。
// *  作者 : 宋志杰 zhijie.song@neuroxess.com
// *  创建日期:   1 2024
// *  修改记录:
// *  <修改者姓名><邮件地址>   <修改内容>
// ******************************************************************************/

#include "CFramelessWidget.h"
#include <QMouseEvent>
#include <QDebug>

FramelessWidget::FramelessWidget(QWidget* parent) : QDialog(parent), strengthArea(8)
{
    cursorShape = this->cursor().shape();
    setMouseTracking(true);// 设置鼠标跟踪，不然只会在鼠标按下时才会触发鼠标移动事件
}

FramelessWidget::~FramelessWidget() = default;

void FramelessWidget::setStrengthDelta(int delta)
{
    strengthArea = delta;
}

void FramelessWidget::enterEvent(QEvent*)
{
    cursorShape = this->cursor().shape();
}

void FramelessWidget::leaveEvent(QEvent*)
{
    this->setCursor(cursorShape);
}

void FramelessWidget::mousePressEvent(QMouseEvent* e)
{
    this->setFocus();
    if (e->button() == Qt::LeftButton)
    {
        clickPos = e->pos();
        if(areaType == Area_Middle && isEnabledDrag(e))
        {
            moving = true;
        }
    }
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        clickPos = e->pos();
        if (areaType == Area_Middle && isEnabledDrag(e))
        {
            moving = false;
        }
    }
}

void FramelessWidget::mouseMoveEvent(QMouseEvent* e)
{
    //qDebug() << e->pos();
    auto button = e->buttons();
    if (e->buttons() & Qt::LeftButton)
    {
        QRect rect = this->geometry();
        const QPoint delta = e->pos() - clickPos;
        switch (areaType)
        {
        case Area_Middle: {
                if(isEnabledDrag(e) && moving)
                {
                    if (const auto a = childAt(e->pos()))
                    {
                        qDebug() << "Mouse move object name: " << a->objectName();
                        qDebug() << "Mouse move devType: " << a->devType();
                    }
                    else
                    {
                        qDebug() << "Mouse move object name NULL ";
                    }
                    move(pos() + delta);
                }
            
        }break;
        case Area_Left: {
            rect.setLeft(rect.left() + delta.x());
            this->setGeometry(rect);
        }break;
        case Area_Right: {
            rect.setRight(rect.right() + delta.x());
            clickPos.setX(clickPos.x() + delta.x());
            this->setGeometry(rect);
        }break;

        case Area_Top: {
            rect.setTop(rect.top() + delta.y());
            this->setGeometry(rect);
        }break;
        case Area_TopLeft: {
            rect.setTopLeft(rect.topLeft() + delta);
            this->setGeometry(rect);
        }break;
        case Area_TopRight: {
            rect.setTopRight(rect.topRight() + delta);
            clickPos.setX(clickPos.x() + delta.x());
            this->setGeometry(rect);
        }break;

        case Area_Bottom: {
            rect.setBottom(rect.bottom() + delta.y());
            clickPos.setY(clickPos.y() + delta.y());
            this->setGeometry(rect);
        }break;
        case Area_BottomLeft: {
            rect.setBottomLeft(rect.bottomLeft() + delta);
            clickPos.setY(clickPos.y() + delta.y());
            this->setGeometry(rect);
        }break;
        case Area_BottomRight: {
            rect.setBottomRight(rect.bottomRight() + delta);
            clickPos += delta;
            this->setGeometry(rect);
        }break;
        case Area_Invalid:
            break;
        }
    }
    else
    {
        areaType = getAreaType(e->pos());
        switch (areaType)
        {
        case Area_TopLeft: {
            this->setCursor(Qt::SizeFDiagCursor);
            break;
        }
        case Area_Left: {
            this->setCursor(Qt::SizeHorCursor);
            break;
        }
        case Area_BottomLeft: {
            this->setCursor(Qt::SizeBDiagCursor);
            break;
        }
        case Area_TopRight: {
            this->setCursor(Qt::SizeBDiagCursor);
            break;
        }
        case Area_Right: {
            this->setCursor(Qt::SizeHorCursor);
            break;
        }
        case Area_BottomRight: {
            this->setCursor(Qt::SizeFDiagCursor);
        }break;
        case Area_Top: {
            this->setCursor(Qt::SizeVerCursor);
            break;
        }
        case Area_Middle: {
            this->setCursor(cursorShape);
            break;
        }
        case Area_Bottom: {
            this->setCursor(Qt::SizeVerCursor);
            break;
        }
        case Area_Invalid:
            break;
        }
    }
}

bool FramelessWidget::isEnabledDrag(QMouseEvent* e)
{
    return true;
}

FramelessWidget::EAreaType FramelessWidget::getAreaType(QPoint point) const
{
    const bool bTop = (point.y() <= strengthArea && point.y() >= -strengthArea);
    const bool bBottom = (point.y() >= height() - strengthArea && point.y() <= height() + strengthArea);
    const bool bLeft = (point.x() <= strengthArea && point.x() >= -strengthArea);
    const bool bRight = (point.x() >= width() - strengthArea && point.x() <= width() + strengthArea);
    const bool bVMid = (point.y() > strengthArea && point.y() < height() - strengthArea);
    const bool bHMid = (point.x() > strengthArea && point.x() < width() - strengthArea);

    if (bHMid)
    {
        if (bVMid)
            return Area_Middle;
        else if (bTop)
            return Area_Top;
        else if (bBottom)
            return Area_Bottom;
    }
    else if (bLeft)
    {
        if (bVMid)
            return Area_Left;
        else if (bTop)
            return Area_TopLeft;
        else if (bBottom)
            return Area_BottomLeft;
    }
    else if (bRight)
    {
        if (bVMid)
            return Area_Right;
        else if (bTop)
            return Area_TopRight;
        else if (bBottom)
            return Area_BottomRight;
    }
    return Area_Invalid;
}

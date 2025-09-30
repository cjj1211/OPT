#pragma execution_character_set("utf-8")
#include "CSwitchButton.h"
#include <QPainter>
BEGIN_NX_NAMESPACE
CSwitchButton::CSwitchButton(QWidget *parent) : QWidget(parent)
{
    setFixedSize(44,25);
    //setStyleSheet("background: transparent;");
    Switchspace = 2;
    Switchradius = 5;
    showText = true;
    showText = false;
    animation = true;

    bgColorOn = QColor(24, 144, 255);
    bgColorOff = QColor(111, 122, 126);

    sliderColorOn = QColor(255, 255, 255,255);
    sliderColorOff = QColor(255, 255, 255,255*0.6);

    textColor = QColor(255, 255, 255);

    textOn = "";
    textOff = "";

    step = 0;
    startX = 0;
    endX = 0;

    timer = new QTimer(this);
    timer->setInterval(30);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateValue()));
}


void CSwitchButton::drawBackGround(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor bgColor = Switchchecked ? bgColorOn : bgColorOff;
 /*   if (isEnabled()) {
        bgColor.setAlpha(100);
    }*/ 

    painter->setBrush(bgColor);

    QRect rect(0, 0, width(), height());
    int side = qMin(width(), height());

    //左侧半圆
    QPainterPath path1;
    path1.addEllipse(rect.x(), rect.y(), side, side);

    //右侧半圆
    QPainterPath path2;
    path2.addEllipse(rect.width() - side, rect.y(), side, side);

    //中间的矩形
    QPainterPath path3;
    path3.addRect(rect.x() + side / 2, rect.y(), rect.width() - side, height());

    QPainterPath path = path1 + path2 + path3;
    painter->drawPath(path);

    //绘制文本

    //滑块半径
    int sliderWidth = qMin(height(), width()) - Switchspace * 2 - 5;
    if (Switchchecked){
        QRect textRect(0, 0, width() - sliderWidth, height());
        painter->setPen(QPen(textColor));
        painter->drawText(textRect, Qt::AlignCenter, textOn);
    } else {
        QRect textRect(sliderWidth, 0, width() - sliderWidth, height());
        painter->setPen(QPen(textColor));
        painter->drawText(textRect, Qt::AlignCenter, textOff);
    }

    painter->restore();
}

void CSwitchButton::drawSlider(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor color = Switchchecked ? sliderColorOn : sliderColorOff;

    painter->setBrush(QBrush(color));

    int sliderWidth = qMin(width(), height()) - Switchspace * 2;
    QRect rect(Switchspace + startX, Switchspace, sliderWidth, sliderWidth);
    painter->drawEllipse(rect);

    painter->restore();
}

void CSwitchButton::paintEvent(QPaintEvent *ev)
{
    //启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制背景
    drawBackGround(&painter);

    //绘制滑块
    drawSlider(&painter);
}

void CSwitchButton::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)

    Switchchecked = !Switchchecked;
    emit statusChanged(Switchchecked);

    //计算步长
    step = width() / 10;

    //计算滑块X轴终点坐标
    if (Switchchecked) {
        endX = width() - height();
    } else {
        endX = 0;
    }

    //判断是否使用动画
    if (animation) {
        timer->start();
    } else{
        startX = endX;
        update();
    }
}

void CSwitchButton::updateValue()
{
    if (Switchchecked) {
        if (startX < endX) {
            startX += step;
        } else {
            startX = endX;
            timer->stop();
        }
    } else {
        if (startX > endX) {
            startX -= step;
        } else {
            startX = endX;
            timer->stop();
        }
    }

    update();
}

int CSwitchButton::SwitchButtonspace() const
{
    return Switchspace;
}

int CSwitchButton::SwitchButtonradius() const
{
    return Switchradius;
}

bool CSwitchButton::SwitchButtonchecked() const
{
    return Switchchecked;
}

bool CSwitchButton::SwitchButtonshowText() const
{
    return showText;
}

bool CSwitchButton::showCircel() const
{
    return showCircle;
}

bool CSwitchButton::SwitchButtonanimation() const
{
    return animation;
}

QColor CSwitchButton::SwitchButtonbgColorOn() const
{
    return bgColorOn;
}

QColor CSwitchButton::SwitchButtonbgColorOff() const
{
    return bgColorOff;
}

QColor CSwitchButton::SwitchButtonsliderColorOn() const
{
    return sliderColorOn;
}

QColor CSwitchButton::SwitchButtonsliderColorOff() const
{
    return sliderColorOff;
}

QColor CSwitchButton::SwitchButtontextColor() const
{
    return textColor;
}

QString CSwitchButton::SwitchButtontextOn() const
{
    return textOn;
}

QString CSwitchButton::SwitchButtontextOff() const
{
    return textOff;
}

int CSwitchButton::SwitchButtonstep() const
{
    return step;
}

int CSwitchButton::SwitchButtonstartX() const
{
    return startX;
}

int CSwitchButton::SwitchButtonendX() const
{
    return endX;
}

void CSwitchButton::setSpace(int space)
{
    if (Switchspace != space) {
        Switchspace = space;
        update();
    }
}

void CSwitchButton::setRadius(int radius)
{
    if (Switchradius != radius) {
        Switchradius = radius;
        update();
    }
}

void CSwitchButton::setChecked(bool checked)
{
    if (Switchchecked != checked) {
        Switchchecked = checked;

        update();
    }
}

void CSwitchButton::setShowText(bool show)
{
    if (showText != show) {
        showText = show;
        update();
    }
}

void CSwitchButton::setShowCircle(bool show)
{
    if (showCircle != show) {
        showCircle = show;
        update();
    }
}

void CSwitchButton::setAnimation(bool ok)
{
    if (animation != ok) {
        animation = ok;
        update();
    }
}

void CSwitchButton::setBgColorOn(const QColor &color)
{
    if (bgColorOn != color) {
        bgColorOn = color;
        update();
    }
}

void CSwitchButton::setBgColorOff(const QColor &color)
{
    if (bgColorOff != color) {
        bgColorOff = color;
        update();
    }
}

void CSwitchButton::setSliderColorOn(const QColor &color)
{
    if (sliderColorOn != color) {
        sliderColorOn = color;
        update();
    }
}

void CSwitchButton::setSliderColorOff(const QColor &color)
{
    if (sliderColorOff != color) {
        sliderColorOff = color;
        update();
    }
}

void CSwitchButton::setTextColor(const QColor &color)
{
    if (textColor != color) {
        textColor = color;
        update();
    }
}

void CSwitchButton::setTextOn(const QString &text)
{
    if (textOn != text) {
        textOn = text;
        update();
    }
}

void CSwitchButton::setTextOff(const QString &text)
{
    if (textOff != text) {
        textOff = text;
        update();
    }
}


END_NX_NAMESPACE
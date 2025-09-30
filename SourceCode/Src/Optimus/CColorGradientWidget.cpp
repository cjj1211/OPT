#include "CColorGradientWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStylePainter>
#include <QPoint>
#include <QPainterPath>
#include <QFontMetrics>
#include <QtCore/qMath.h>

BEGIN_NX_NAMESPACE
CColorGradientWidget::CColorGradientWidget(GradientType gradientType, const QString& gradientTitle, QWidget *parent)
    : QWidget(parent)
    , type(gradientType)
    , title(gradientTitle)
    , leftBtnPos(0.33)
    , rightBtnPos(0.66)
    , btnWidth(14)
    , btnHeight(34)
    , startColor(QColor::fromHsv(280, 255,255))
    , leftBtnColor(QColor::fromHsv(210, 255, 255))
    , rightBtnColor(QColor::fromHsv(40, 255, 255))
    , endColor(QColor::fromHsv(0, 255, 255))
    , backgroundColor("#283C82")
    , borderColor("#FFFFFF")
    , isLeftBtnPressed(false)
    , isRightBtnPressed(false)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
}

CColorGradientWidget::~CColorGradientWidget()
{}

QColor CColorGradientWidget::getColor(const double value) const
{
    double hue = 0;
    if (value >= 1)
    {
        return endColor;
    }

    if (value <= 0)
    {
        return startColor;
    }

    if (value < leftBtnPos)
    {
        double leftHue = leftBtnColor.hue();
        double startHue = startColor.hue();
        hue = value / leftBtnPos * (leftHue - startHue) + startHue;
    }
    else if (value >= leftBtnPos && value < rightBtnPos)
    {
        double leftHue = leftBtnColor.hue();
        double rightHue = rightBtnColor.hue();
        hue = (value - leftBtnPos) / (rightBtnPos - leftBtnPos) * (rightHue - leftHue) + leftHue;
    }
    else
    {
        double rightHue = rightBtnColor.hue();
        double endHue = endColor.hue();
        hue = (value - rightBtnPos) / (1.0 - rightBtnPos) * (endHue - rightHue) + rightHue;
    }

    return QColor::fromHsv(hue, 255, 255);
}

void CColorGradientWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (isLeftBtnPressed || isRightBtnPressed)
    {
        auto pos = static_cast<double>(event->pos().x() - scopeFrame.x())/ static_cast<double>(scopeFrame.width());
        if (isLeftBtnPressed && 
            event->pos().x() + btnWidth< rightBtnFrame.left())
        {
            if (pos < 0) pos = 0;
            leftBtnPos = pos;
            update();
        }
        else if (isRightBtnPressed && 
            event->pos().x() - btnWidth > leftBtnFrame.right() )
        {
            if (pos > 1) { pos = 1; }
            rightBtnPos = pos;
            update();
        }
    }
}

void CColorGradientWidget::mousePressEvent(QMouseEvent* event)
{
    if (leftBtnFrame.contains(event->pos()))
    {
        isLeftBtnPressed = true;
        update();
    }
    else if (rightBtnFrame.contains(event->pos()))
    {
        isRightBtnPressed = true;
        update();
    }
}

void CColorGradientWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (isLeftBtnPressed || isRightBtnPressed) {
        isLeftBtnPressed = false;
        isRightBtnPressed = false;
        update();
        emit signalGradientChanged();
    }
}

void CColorGradientWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(&image);
    QRect imageFrame(rect());
    painter.setRenderHint(QPainter::Antialiasing, true);
    auto oldBorder = borderColor;
    borderColor = "Gray";
    drawRectWithBorder(painter, imageFrame, backgroundColor, 4);
    borderColor = oldBorder;

    draw(painter, imageFrame);

    QStylePainter stylePainter(this);
    stylePainter.drawImage(0, 0, image);
}

void CColorGradientWidget::resizeEvent(QResizeEvent* event)
{
    image = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    update();
}

void CColorGradientWidget::draw(QPainter& painter, QRect& imageFrame)
{
    int leftMargin = 45;
    int rightMargin = 45;
    int rectHeight = 18;
    auto centerY = (imageFrame.top() + imageFrame.bottom()) / 2;
    scopeFrame = QRect(imageFrame.left() + leftMargin, centerY- rectHeight / 2, imageFrame.width() - leftMargin - rightMargin, rectHeight);

    drawTitle(painter);
    if (type == GT_IMPEDANCE) {
        drawImpedanceTags(painter);
    }
    else {
        drawHeatTags(painter);
    }

    QBrush brush(setupGradient(scopeFrame));
    painter.setBrush(brush);
    painter.drawRect(scopeFrame);

    double borderRadius = std::min(btnWidth, btnHeight) / 2;
    // 绘制按钮
    double leftBtnX = static_cast<double>(scopeFrame.width()) * leftBtnPos + scopeFrame.x();
    leftBtnFrame = QRectF(leftBtnX - btnWidth / 2, centerY - btnHeight / 2, btnWidth, btnHeight);
    if (isLeftBtnPressed)
    {
        drawRectWithBorder(painter, leftBtnFrame, QColor::fromHsv(leftBtnColor.hue(),150, 255), borderRadius);
    }
    else
    {
        drawRectWithBorder(painter, leftBtnFrame, leftBtnColor, borderRadius);
    }
    
    double rightBtnX = static_cast<double>(scopeFrame.width()) * rightBtnPos + scopeFrame.x();
    rightBtnFrame = QRectF(rightBtnX - btnWidth / 2, centerY - btnHeight / 2, btnWidth, btnHeight);
    if (isRightBtnPressed)
    {
        drawRectWithBorder(painter, rightBtnFrame, QColor::fromHsv(rightBtnColor.hue(), 150, 255), borderRadius);
    }
    else
    {
        drawRectWithBorder(painter, rightBtnFrame, rightBtnColor, borderRadius);
    }

}

void CColorGradientWidget::drawRectWithBorder(QPainter& painter, const QRectF& frame, const QColor& color, const double borderRadius)
{
    QPainterPath path;
    // 设置起点为矩形左上圆角圆心
    auto radiusCircleSize = QSizeF(borderRadius * 2, borderRadius * 2);
    path.moveTo(frame.topLeft().x() + borderRadius, frame.topLeft().y());
    // 绘制圆角 圆弧以外切圆的90度位置为起点，逆时针画圆弧运行90度结束（从12点钟方向 - 9点钟方向） 
    path.arcTo(QRectF(frame.topLeft(), radiusCircleSize), 90, 90);
    path.lineTo(frame.bottomLeft().x(), frame.bottomLeft().y() - borderRadius);
    path.arcTo(QRectF(QPointF(frame.bottomLeft().x(), frame.bottomLeft().y() - (borderRadius * 2)), radiusCircleSize), 180, 90);
    path.lineTo(frame.bottomRight().x() - borderRadius, frame.bottomRight().y());
    path.arcTo(QRectF(QPointF(frame.bottomRight().x() - (borderRadius * 2), frame.bottomRight().y() - (borderRadius * 2)), radiusCircleSize), 270, 90);
    path.lineTo(frame.topRight().x(), frame.topRight().y() + borderRadius);
    path.arcTo(QRectF(QPointF(frame.topRight().x() - (borderRadius * 2), frame.topRight().y()), QSizeF(borderRadius * 2, borderRadius * 2)), 0, 90);
    path.lineTo(frame.topLeft().x() + borderRadius, frame.topLeft().y());
    painter.setBrush(QBrush(color));
    painter.fillPath(path, QBrush(color));
    painter.setPen(QPen(borderColor, 2));
    painter.drawPath(path);
}

void CColorGradientWidget::drawTitle(QPainter& painter)
{
    QFont font;
    font.setWeight(55);
    font.setFamily("Microsoft YaHei");
    font.setPixelSize(20);
    painter.setFont(font);

    auto labelFontMetrics = new QFontMetrics(font);
    auto width = labelFontMetrics->horizontalAdvance(title);
    auto height = labelFontMetrics->height();
    auto titlePosition = QPoint(rect().center().x() - width / 2, rect().bottom() - height / 2 -4);
    painter.setPen(QPen(QColor(255, 255, 255, 204), 2));
    painter.drawText(titlePosition, title);
}

void CColorGradientWidget::drawImpedanceTags(QPainter& painter)
{
    QMap<QString, float> tagsMap({ {QString::fromLocal8Bit("1kΩ"),0},
        //{QString::fromLocal8Bit("10kΩ"), 0.25},
        {QString::fromLocal8Bit("100kΩ"), 0.5},
        //{QString::fromLocal8Bit("1MΩ"), 0.75},
        {QString::fromLocal8Bit("10MΩ"), 1.0} });

    QFont font;
    font.setWeight(55);
    font.setFamily("Microsoft YaHei");
    font.setPixelSize(18);
    painter.setFont(font);
    auto labelFontMetrics = new QFontMetrics(font);
    painter.setPen(QPen(QColor(255, 255, 255, 204), 1));

    double height = labelFontMetrics->height();
    double textBottom = scopeFrame.top() - 13;
    for (auto& tag : tagsMap.keys())
    {
        double pos = tagsMap[tag];
        double width = labelFontMetrics->horizontalAdvance(tag);
        double textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * pos;
        
        painter.drawText(QPoint(textCenterX - width / 2, textBottom), tag);
        painter.drawLine(QLine(textCenterX, scopeFrame.top(), textCenterX, textBottom + 6));
    }

    double maxValue = 10000000;
    double minValue = 1000;
    double mangnitude = qPow(10, leftBtnPos * qLn(maxValue/ minValue) / qLn(10)) * minValue;
    QString magnitudeStr = "";
    if (mangnitude >= 1000000) {
        magnitudeStr = QString::number(mangnitude / 1000000.0, 'f', 1) + QString::fromLocal8Bit("MΩ");
    }
    else if (mangnitude >= 1000) {
        magnitudeStr = QString::number(mangnitude / 1000.0, 'f', 1) + QString::fromLocal8Bit("kΩ");
    }
    else {
        magnitudeStr = QString::number(mangnitude) + QString::fromLocal8Bit("Ω");
    }


    QString leftTag = magnitudeStr;
    double width = labelFontMetrics->horizontalAdvance(leftTag);
    double textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * leftBtnPos;
    painter.fillRect(QRectF(textCenterX - width / 2, textBottom - height, width, height), backgroundColor);
    painter.drawText(QPoint(textCenterX - width / 2, textBottom), leftTag);

    mangnitude = qPow(10, rightBtnPos * qLn(maxValue / minValue) / qLn(10)) * minValue;
    magnitudeStr = "";
    if (mangnitude >= 1000000) {
        magnitudeStr = QString::number(mangnitude / 1000000.0, 'f', 1) + QString::fromLocal8Bit("MΩ");
    }
    else if (mangnitude >= 1000) {
        magnitudeStr = QString::number(mangnitude / 1000.0, 'f', 1) + QString::fromLocal8Bit("kΩ");
    }
    else {
        magnitudeStr = QString::number(mangnitude) + QString::fromLocal8Bit("Ω");
    }
    QString rightTag = magnitudeStr;
    width = labelFontMetrics->horizontalAdvance(rightTag);
    textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * rightBtnPos;
    painter.fillRect(QRectF(textCenterX - width / 2, textBottom - height, width, height), backgroundColor);
    painter.drawText(QPoint(textCenterX - width / 2, textBottom), rightTag);
}

void CColorGradientWidget::drawHeatTags(QPainter& painter)
{
    QMap<QString, float> tagsMap({ {QString::fromLocal8Bit("0"),0},
        {QString::fromLocal8Bit("0.5"), 0.5},
        {QString::fromLocal8Bit("1"), 1.0} });

    QFont font;
    font.setWeight(55);
    font.setFamily("Microsoft YaHei");
    font.setPixelSize(18);
    painter.setFont(font);
    auto labelFontMetrics = new QFontMetrics(font);
    painter.setPen(QPen(QColor(255, 255, 255, 204), 1));

    double height = labelFontMetrics->height();
    double textBottom = scopeFrame.top() - 13;
    for (auto& tag : tagsMap.keys())
    {
        double pos = tagsMap[tag];
        double width = labelFontMetrics->horizontalAdvance(tag);
        double textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * pos;

        painter.drawText(QPoint(textCenterX - width / 2, textBottom), tag);
        painter.drawLine(QLine(textCenterX, scopeFrame.top(), textCenterX, textBottom + 6));
    }

    QString leftTag = QString::number(leftBtnPos, 'f', 2);
    double width = labelFontMetrics->horizontalAdvance(leftTag);
    double textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * leftBtnPos;
    painter.fillRect(QRectF(textCenterX - width / 2, textBottom - height, width, height), backgroundColor);
    painter.drawText(QPoint(textCenterX - width / 2, textBottom), leftTag);

    QString rightTag = QString::number(rightBtnPos, 'f', 2);
    width = labelFontMetrics->horizontalAdvance(rightTag);
    textCenterX = scopeFrame.left() + static_cast<double>(scopeFrame.width()) * rightBtnPos;
    painter.fillRect(QRectF(textCenterX - width / 2, textBottom - height, width, height), backgroundColor);
    painter.drawText(QPoint(textCenterX - width / 2, textBottom), rightTag);
}

// Set up a QLinearGradient representing the portion of the rainbow from purple to red that we want to represent impedances from 10K to 10M.
QLinearGradient CColorGradientWidget::setupGradient(const QRectF& frame) {
    QLinearGradient rainbow(frame.topLeft(), frame.bottomRight());
    auto setGradient = [&](const double startHue,const double endHue, const double startPos, const double endPos) {
        for (double i = 0; i < 100.0; ++i)
        {
            const double fractionalI = i / 100.0;
            const double hue = startHue + fractionalI * (endHue - startHue);
            rainbow.setColorAt(startPos + fractionalI * (endPos - startPos), QColor::fromHsv(hue, 255, 255));
        }
    };

    setGradient(startColor.hue(), leftBtnColor.hue(), 0, leftBtnPos);
    setGradient(leftBtnColor.hue(), rightBtnColor.hue(), leftBtnPos, rightBtnPos);
    setGradient(rightBtnColor.hue(), endColor.hue(), rightBtnPos, 1.0);
    return rainbow;
}
END_NX_NAMESPACE
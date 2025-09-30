#include "CPlotDecorator.h"


#include <qmath.h>
#include <QPainter>

#include "CCoordinateTranslator.h"

CPlotDecorator::CPlotDecorator(QPainter& painter_) :
    painter(painter_)
{
}

void CPlotDecorator::drawVerticalAxisLine(CCoordinateTranslator& ct, double xReal, int shrink) const
{
    painter.drawLine(QPoint(ct.screenXFromRealX(xReal), ct.yTop() + shrink),
        QPoint(ct.screenXFromRealX(xReal), ct.yBottom() - shrink));
}

void CPlotDecorator::drawHorizontalAxisLine(CCoordinateTranslator& ct, double yReal, int shrink) const
{
    painter.drawLine(QPoint(ct.xLeft() + shrink, ct.screenYFromRealY(yReal)),
        QPoint(ct.xRight() - shrink, ct.screenYFromRealY(yReal)));
}

void CPlotDecorator::writeLabel(const QString& text, int x, int y, int flags) const
{
    const int TextBoxWidth = painter.fontMetrics().horizontalAdvance(text);
    const int TextBoxHeight = painter.fontMetrics().height();
    const int TextBoxHalfWidth = qCeil((double)TextBoxWidth / 2.0);
    const int TextBoxHalfHeight = qCeil((double)TextBoxHeight / 2.0);

    int xPos = x;
    int yPos = y;

    if (flags & Qt::AlignHCenter) {
        xPos -= TextBoxHalfWidth;
    }
    else if (flags & Qt::AlignRight) {
        xPos -= TextBoxWidth;
    }

    if (flags & Qt::AlignVCenter) {
        yPos -= TextBoxHalfHeight;
    }
    else if (flags & Qt::AlignBottom) {
        yPos -= TextBoxHeight;
    }

    painter.drawText(xPos, yPos, TextBoxWidth, TextBoxHeight, flags, text);
}

void CPlotDecorator::writeLabel(int number, int x, int y, int flags) const
{
    writeLabel(QString::number(number), x, y, flags);
}

void CPlotDecorator::writeYAxisLabel(const QString& text, CCoordinateTranslator& ct, int xOffset) const
{
    painter.save();
    painter.translate(ct.xLeft() - xOffset, (ct.yBottom() + ct.yTop()) / 2);
    painter.rotate(-90);
    // writeLabel(text, ct.xLeft() - xOffset, (ct.yBottom() + ct.yTop()) / 2, Qt::AlignHCenter | Qt::AlignBottom);
    writeLabel(text, 0, 0, Qt::AlignHCenter | Qt::AlignBottom);
    painter.restore();
}

void CPlotDecorator::drawTickMarkLeft(CCoordinateTranslator& ct, double yReal, int length) const
{
    painter.drawLine(QPoint(ct.xLeft() - 1, ct.screenYFromRealY(yReal)),
        QPoint(ct.xLeft() - 1 - length, ct.screenYFromRealY(yReal)));
}

void CPlotDecorator::drawTickMarkRight(CCoordinateTranslator& ct, double yReal, int length) const
{
    painter.drawLine(QPoint(ct.xRight() + 1, ct.screenYFromRealY(yReal)),
        QPoint(ct.xRight() + 1 + length, ct.screenYFromRealY(yReal)));
}

void CPlotDecorator::drawTickMarkBottom(CCoordinateTranslator& ct, double xReal, int length) const
{
    painter.drawLine(QPoint(ct.screenXFromRealX(xReal), ct.yBottom() + 1),
        QPoint(ct.screenXFromRealX(xReal), ct.yBottom() + 1 + length));
}

void CPlotDecorator::drawTickMarkTop(CCoordinateTranslator& ct, double xReal, int length) const
{
    painter.drawLine(QPoint(ct.screenXFromRealX(xReal), ct.yTop() - 1),
        QPoint(ct.screenXFromRealX(xReal), ct.yTop() - 1 - length));
}

void CPlotDecorator::drawLabeledTickMarkLeft(const QString& text, CCoordinateTranslator& ct, double yReal, int length) const
{
    if (length != 0) drawTickMarkLeft(ct, yReal, length);
    writeLabel(text, ct.xLeft() - length - 3, ct.screenYFromRealY(yReal), Qt::AlignRight | Qt::AlignVCenter);
}

void CPlotDecorator::drawLabeledTickMarkLeft(int number, CCoordinateTranslator& ct, double yReal, int length) const
{
    drawLabeledTickMarkLeft(QString::number(number), ct, yReal, length);
}

void CPlotDecorator::drawLabeledTickMarkRight(const QString& text, CCoordinateTranslator& ct, double yReal, int length) const
{
    if (length != 0) drawTickMarkRight(ct, yReal, length);
    writeLabel(text, ct.xRight() + length + 4, ct.screenYFromRealY(yReal), Qt::AlignLeft | Qt::AlignVCenter);
}

void CPlotDecorator::drawLabeledTickMarkRight(int number, CCoordinateTranslator& ct, double yReal, int length) const
{
    drawLabeledTickMarkRight(QString::number(number), ct, yReal, length);
}

void CPlotDecorator::drawLabeledTickMarkBottom(const QString& text, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd) const
{
    if (length != 0) drawTickMarkBottom(ct, xReal, length);
    writeLabel(text, ct.screenXFromRealX(xReal), ct.yBottom() + length + 2,
        rightEnd ? (Qt::AlignRight | Qt::AlignTop) : (Qt::AlignHCenter | Qt::AlignTop));
}

void CPlotDecorator::drawLabeledTickMarkBottom(int number, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd) const
{
    drawLabeledTickMarkBottom(QString::number(number), ct, xReal, length, rightEnd);
}

void CPlotDecorator::drawLabeledTickMarkTop(const QString& text, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd) const
{
    if (length != 0) drawTickMarkTop(ct, xReal, length);
    writeLabel(text, ct.screenXFromRealX(xReal), ct.yTop() - length - 2,
        rightEnd ? (Qt::AlignRight | Qt::AlignBottom) : (Qt::AlignHCenter | Qt::AlignBottom));
}

void CPlotDecorator::drawLabeledTickMarkTop(int number, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd) const
{
    drawLabeledTickMarkTop(QString::number(number), ct, xReal, length, rightEnd);
}

// Given the maximum y value in a data series (maxYValue), this function generates a appropriate y scale ranging from zero
// to a 'round' number slightly higher than maxYValue; this value is returned.  The function also returns two vectors: a
// list of values for evenly-spaced tick marks on this axis and a list of QString labels for these tick marks.
double CPlotDecorator::autoCalculateYAxis(double maxY, std::vector<double>& yAxisTicks, std::vector<QString>& yAxisLabels) const
{
    yAxisTicks.clear();
    yAxisLabels.clear();
    yAxisTicks.push_back(0.0);
    yAxisLabels.push_back("0");

    if (maxY <= 0.0) return 0.0;

    double magnitude = pow(10.0, floor(log10(maxY)));
    double significand = maxY / magnitude;

    if (significand > 8.0) {
        yAxisTicks.push_back(2.0 * magnitude);
        yAxisTicks.push_back(4.0 * magnitude);
        yAxisTicks.push_back(6.0 * magnitude);
        yAxisTicks.push_back(8.0 * magnitude);
        yAxisTicks.push_back(10.0 * magnitude);
    }
    else if (significand > 5.0) {
        yAxisTicks.push_back(2.0 * magnitude);
        yAxisTicks.push_back(4.0 * magnitude);
        yAxisTicks.push_back(6.0 * magnitude);
        yAxisTicks.push_back(8.0 * magnitude);
    }
    else if (significand > 4.0) {
        yAxisTicks.push_back(1.0 * magnitude);
        yAxisTicks.push_back(2.0 * magnitude);
        yAxisTicks.push_back(3.0 * magnitude);
        yAxisTicks.push_back(4.0 * magnitude);
        yAxisTicks.push_back(5.0 * magnitude);
    }
    else if (significand > 2.5) {
        yAxisTicks.push_back(1.0 * magnitude);
        yAxisTicks.push_back(2.0 * magnitude);
        yAxisTicks.push_back(3.0 * magnitude);
        yAxisTicks.push_back(4.0 * magnitude);
    }
    else if (significand > 2.0) {
        yAxisTicks.push_back(0.5 * magnitude);
        yAxisTicks.push_back(1.0 * magnitude);
        yAxisTicks.push_back(1.5 * magnitude);
        yAxisTicks.push_back(2.0 * magnitude);
        yAxisTicks.push_back(2.5 * magnitude);
    }
    else if (significand > 1.5) {
        yAxisTicks.push_back(0.5 * magnitude);
        yAxisTicks.push_back(1.0 * magnitude);
        yAxisTicks.push_back(1.5 * magnitude);
        yAxisTicks.push_back(2.0 * magnitude);
    }
    else {
        yAxisTicks.push_back(0.2 * magnitude);
        yAxisTicks.push_back(0.4 * magnitude);
        yAxisTicks.push_back(0.6 * magnitude);
        yAxisTicks.push_back(0.8 * magnitude);
        yAxisTicks.push_back(1.0 * magnitude);
        yAxisTicks.push_back(1.2 * magnitude);
    }

    for (int i = 1; i < (int)yAxisTicks.size(); ++i) {
        yAxisLabels.push_back(QString::number(yAxisTicks[i]));
    }
    return yAxisTicks[yAxisTicks.size() - 1];
}

MinMaxPair CPlotDecorator::autoCalculateLogYAxis(double minNonZeroY, double maxY, std::vector<double>& yAxisTicks,
    std::vector<QString>& yAxisLabels) const
{
    MinMaxPair result;
    result.min = floor(log10(minNonZeroY));
    result.max = ceil(log10(maxY));
    for (int i = round(result.min); i <= round(result.max); ++i) {
        yAxisTicks.push_back(i);
        yAxisLabels.push_back(QString::number(pow(10.0, i), 'f', (i < 0) ? -i : 0));
    }
    return result;
}
#include "CColorScale.h"

BEGIN_NX_NAMESPACE

CColorScale::CColorScale(float minValue_, float maxValue_) :
    minValue(minValue_),
    maxValue(maxValue_)
{
    valueRange = maxValue - minValue;
    colorMap.resize(ColorMapSize);
    calculateColorMap();
}

void CColorScale::setRange(float minValue_, float maxValue_)
{
    minValue = minValue_;
    maxValue = maxValue_;
    valueRange = maxValue - minValue;
    calculateColorMap();
}

QColor CColorScale::getColor(double value) const
{
    int index = qRound((ColorMapSize - 1) * (value - minValue) / valueRange);
    if (index < 0) {
        index = 0;
    }
    else if (index >= ColorMapSize) {
        index = ColorMapSize - 1;
    }
    return colorMap[index];
}

void CColorScale::copyColorMapToArray(std::vector<std::vector<float> >& mapArray) const
{
    for (int i = 0; i < (int)mapArray.size(); ++i) {
        mapArray[i].clear();
    }
    mapArray.clear();

    mapArray.resize(ColorMapSize);
    for (int i = 0; i < ColorMapSize; ++i) {
        mapArray[i].resize(3);
        mapArray[i][0] = colorMap[i].redF();
        mapArray[i][1] = colorMap[i].greenF();
        mapArray[i][2] = colorMap[i].blueF();
    }
}

void CColorScale::drawColorScale(QPainter& painter, const QRect& r) const
{
    int yTop = r.top();
    int yBottom = r.bottom();
    int xLeft = r.left();
    int xRight = r.right();

    for (int y = yTop; y <= yBottom; ++y) {
        int index = qRound((double)((ColorMapSize - 1) * (yBottom - y)) / (double)(yBottom - yTop));
        painter.setPen(colorMap[index]);
        painter.drawLine(QLine(xLeft, y, xRight, y));
    }
}

void CColorScale::calculateColorMap()
{
    int c1 = qRound((double)ColorMapSize * 0.20);
    int c2 = qRound((double)ColorMapSize * 0.75);
    double iNorm, hue, value, saturation;

    // Dark violet fading to bright blue
    for (int i = 0; i < c1; ++i) {
        iNorm = (double)i / (double)c1;
        hue = 290.0 - 40.0 * iNorm;
        saturation = 255.0;
        value = 255.0 * (0.7 * iNorm + 0.3);
        colorMap[i] = QColor::fromHsv(hue, saturation, value);
    }
    // Hue sweeping from blue through green, yellow, orange, to red
    for (int i = c1; i < c2; ++i) {
        iNorm = (double)(i - c1) / (double)(c2 - c1 + 1);
        hue = 250.0 - 255.0 * iNorm;
        saturation = 255.0;
        value = 255.0;
        if (hue < 0.0) hue += 360.0;
        colorMap[i] = QColor::fromHsv(hue, saturation, value);
    }
    // Red fading into white
    for (int i = c2; i < ColorMapSize; ++i) {
        iNorm = (double)(i - c2) / (double)(ColorMapSize - c2 - 1);
        saturation = 255.0 * (1.0 - iNorm);
        value = 255.0;
        hue = 355.0;
        colorMap[i] = QColor::fromHsv(hue, saturation, value);
    }
}

END_NX_NAMESPACE
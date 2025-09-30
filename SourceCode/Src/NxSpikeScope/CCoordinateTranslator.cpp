#include "CCoordinateTranslator.h"



CCoordinateTranslator::CCoordinateTranslator()
{
    set(QRect(0, 0, 2, 2), 0.0, 1.0, 0.0, 1.0);   // dummy default values to avoid divide-by zero errors
}

CCoordinateTranslator::CCoordinateTranslator(const QRect frame__, const double xMinReal__, const double xMaxReal__, const double yMinReal__, const double yMaxReal__)
{
    set(frame__, xMinReal__, xMaxReal__, yMinReal__, yMaxReal__);
}

void CCoordinateTranslator::set(const QRect frame__, const double xMinReal__, const double xMaxReal__, const double yMinReal__, const double yMaxReal__)
{
    frame_ = frame__;
    xMinReal_ = xMinReal__;
    xMaxReal_ = xMaxReal__;
    yMinReal_ = yMinReal__;
    yMaxReal_ = yMaxReal__;
    double xSpanReal = xMaxReal_ - xMinReal_;
    double ySpanReal = yMaxReal_ - yMinReal_;
    double widthScreen = (double)(frame_.width() - 1);
    double heightScreen = (double)(frame_.height() - 1);

    if (xSpanReal != 0.0) xScale = widthScreen / xSpanReal;
    else xScale = std::numeric_limits<double>::infinity();  // Prevent divide-by-zero error.

    if (ySpanReal != 0.0) yScale = heightScreen / ySpanReal;
    else yScale = std::numeric_limits<double>::infinity();  // Prevent divide-by-zero error.
}

int CCoordinateTranslator::screenXFromRealX(const double x) const
{
    return frame_.left() + (int)(xScale * (x - xMinReal_));
}

int CCoordinateTranslator::screenYFromRealY(const double y) const
{
    return frame_.top() + (int)(yScale * (yMaxReal_ - y));
}

double CCoordinateTranslator::realXFromScreenX(const int x) const
{
    return xMinReal_ + ((double)(x - frame_.left())) / xScale;
}

double CCoordinateTranslator::realYFromScreenY(const int y) const
{
    return yMaxReal_ - ((double)(y - frame_.top())) / yScale;
}

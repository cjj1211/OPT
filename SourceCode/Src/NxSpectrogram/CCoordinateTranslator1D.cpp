#include "CCoordinateTranslator1D.h"

#include <limits>

BEGIN_NX_NAMESPACE
CCoordinateTranslator1D::CCoordinateTranslator1D()
{
    set(0, 1, 0.0, 1.0);   // dummy default values to avoid divide-by zero errors
}

CCoordinateTranslator1D::CCoordinateTranslator1D(int minScreen__, int maxScreen__, double minReal__, double maxReal__)
{
    set(minScreen__, maxScreen__, minReal__, maxReal__);
}

void CCoordinateTranslator1D::set(int minScreen__, int maxScreen__, double minReal__, double maxReal__)
{
    minScreen_ = minScreen__;
    maxScreen_ = maxScreen__;
    minReal_ = minReal__;
    maxReal_ = maxReal__;
    double spanReal = maxReal_ - minReal_;
    double spanScreen = (double)(maxScreen_ - minScreen_);

    if (spanReal != 0.0) scale = spanScreen / spanReal;
    else scale = std::numeric_limits<double>::infinity();  // Prevent divide-by-zero error.
}

int CCoordinateTranslator1D::screenFromReal(double real) const
{
    return minScreen_ + (int)(scale * (real - minReal_));
}

double CCoordinateTranslator1D::realFromScreen(int screen) const
{
    return minReal_ + ((double)(screen - minScreen_)) / scale;
}
END_NX_NAMESPACE
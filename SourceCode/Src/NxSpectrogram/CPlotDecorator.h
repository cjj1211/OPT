/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   October 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/

#ifndef __C_PLOT_DECORATOR_H
#define __C_PLOT_DECORATOR_H

#include "CCoordinateTranslator.h"
#include <QPainter>
#include <vector>

BEGIN_NX_NAMESPACE

struct MinMaxPair
{
    double min;
    double max;
};

class CPlotDecorator
{
public:
    CPlotDecorator(QPainter& painter_);

    void drawVerticalAxisLine(CCoordinateTranslator& ct, double xReal, int shrink = 0) const;
    void drawHorizontalAxisLine(CCoordinateTranslator& ct, double yReal, int shrink = 0) const;

    void writeLabel(const QString& text, int x, int y, int flags) const;
    void writeLabel(int number, int x, int y, int flags) const;

    void writeYAxisLabel(const QString& text, CCoordinateTranslator& ct, int xOffset) const;

    void drawTickMarkLeft(CCoordinateTranslator& ct, double yReal, int length) const;
    void drawTickMarkRight(CCoordinateTranslator& ct, double yReal, int length) const;
    void drawTickMarkBottom(CCoordinateTranslator& ct, double xReal, int length) const;
    void drawTickMarkTop(CCoordinateTranslator& ct, double xReal, int length) const;

    void drawLabeledTickMarkLeft(const QString& text, CCoordinateTranslator& ct, double yReal, int length) const;
    void drawLabeledTickMarkLeft(int number, CCoordinateTranslator& ct, double yReal, int length) const;
    void drawLabeledTickMarkRight(const QString& text, CCoordinateTranslator& ct, double yReal, int length) const;
    void drawLabeledTickMarkRight(int number, CCoordinateTranslator& ct, double yReal, int length) const;
    void drawLabeledTickMarkBottom(const QString& text, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd = false) const;
    void drawLabeledTickMarkBottom(int number, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd = false) const;
    void drawLabeledTickMarkTop(const QString& text, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd = false) const;
    void drawLabeledTickMarkTop(int number, CCoordinateTranslator& ct, double xReal, int length, bool rightEnd = false) const;

    double autoCalculateYAxis(double maxY, std::vector<double>& yAxisTicks, std::vector<QString>& yAxisLabels) const;
    MinMaxPair autoCalculateLogYAxis(double minNonZeroY, double maxY, std::vector<double>& yAxisTicks,
        std::vector<QString>& yAxisLabels) const;

private:
    QPainter& painter;
};

END_NX_NAMESPACE
#endif
/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __NX_WAVEFORM_WIDGET_GLOBAL_H
#define __NX_WAVEFORM_WIDGET_GLOBAL_H
#include <CLogger.h>
#include <QtCore/qglobal.h>
#include <QString>
#include <QStringList>
#include <string>
#include <ranges>
#include <map>

#ifndef BUILD_STATIC
# if defined(NXWAVEFORMWIDGET_LIB)
#  define NX_WAVEFORM_WIDGET_EXPORT Q_DECL_EXPORT
# else
#  define NX_WAVEFORM_WIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_WAVEFORM_WIDGET_EXPORT
#endif

using namespace NX;

const int MaxNumWaveformsInGroup = 4;

// Special Unicode characters, as QString data type
const QString DeltaSymbol = QString((QChar)0x0394);
const QString MuSymbol = QString((QChar)0x03bc);
const QString MicroVoltsSymbol = MuSymbol + QString("V");
const QString MicroAmpsSymbol = MuSymbol + QString("A");
const QString MicroSecondsSymbol = MuSymbol + QString("s");
const QString OmegaSymbol = QString((QChar)0x03a9);
const QString AngleSymbol = QString((QChar)0x2220);
const QString DegreeSymbol = QString((QChar)0x00b0);
const QString PlusMinusSymbol = QString((QChar)0x00b1);
const QString SqrtSymbol = QString((QChar)0x221a);
const QString EnDashSymbol = QString((QChar)0x2013);
const QString EmDashSymbol = QString((QChar)0x2014);
const QString EllipsisSymbol = QString((QChar)0x2026);
const QString CopyrightSymbol = QString((QChar)0x00a9);

//const QString MuSymbolTCP = QString::fromStdWString(L"\u00b5");
//const QString PlusMinusSymbolTCP = QString::fromStdWString(L"\u00b1");

const QString EndOfLine = QString("\n");

enum WaveformType {
    AmplifierWaveform,
    UnknownWaveform,
    WaveformDivider
};

enum YScaleType {
	WidebandYScale,
	UnknownYScale
};

#endif
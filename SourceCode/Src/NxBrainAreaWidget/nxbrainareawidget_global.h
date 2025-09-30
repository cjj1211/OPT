#pragma once

#include <QtCore/qglobal.h>
#include <CLogger.h>

using namespace NX;

#ifndef BUILD_STATIC
# if defined(NXBRAINAREAWIDGET_LIB)
#  define NX_BRAIN_AREA_WIDGET_EXPORT Q_DECL_EXPORT
# else
#  define NX_BRAIN_AREA_WIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_BRAIN_AREA_WIDGET_EXPORT
#endif

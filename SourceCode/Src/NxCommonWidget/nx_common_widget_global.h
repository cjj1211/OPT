#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXCOMMONWIDGET_LIB)
#  define NX_COMMON_WIDGET_EXPORT Q_DECL_EXPORT
# else
#  define NX_COMMON_WIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_COMMON_WIDGET_EXPORT
#endif

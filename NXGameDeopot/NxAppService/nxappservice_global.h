#pragma once

#include <QtCore/qglobal.h>
#include <CLogger.h>
using namespace NX;
#ifndef BUILD_STATIC
# if defined(NXAPPSERVICE_LIB)
#  define NXAPPSERVICE_EXPORT Q_DECL_EXPORT
# else
#  define NXAPPSERVICE_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXAPPSERVICE_EXPORT
#endif

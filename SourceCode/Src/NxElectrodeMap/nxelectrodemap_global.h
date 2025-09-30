#pragma once
#include <QtCore/qglobal.h>
#include <CLogger.h>
using namespace NX;
#ifndef BUILD_STATIC
# if defined(NXELECTRODEMAP_LIB)
#  define NXELECTRODEMAP_EXPORT Q_DECL_EXPORT
# else
#  define NXELECTRODEMAP_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXELECTRODEMAP_EXPORT
#endif

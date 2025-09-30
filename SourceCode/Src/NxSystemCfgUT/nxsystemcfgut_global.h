#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXSYSTEMCFGUT_LIB)
#  define NXSYSTEMCFGUT_EXPORT Q_DECL_EXPORT
# else
#  define NXSYSTEMCFGUT_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXSYSTEMCFGUT_EXPORT
#endif

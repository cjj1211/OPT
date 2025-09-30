#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXDBMANAGER_LIB)
#  define NXDBMANAGER_EXPORT Q_DECL_EXPORT
# else
#  define NXDBMANAGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXDBMANAGER_EXPORT
#endif

#pragma once

#include <QtCore/qglobal.h>
#include <CLogger.h>

#ifndef BUILD_STATIC
# if defined(NXCOMMUNICATION_LIB)
#  define NXCOMMUNICATION_EXPORT Q_DECL_EXPORT
# else
#  define NXCOMMUNICATION_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXCOMMUNICATION_EXPORT
#endif

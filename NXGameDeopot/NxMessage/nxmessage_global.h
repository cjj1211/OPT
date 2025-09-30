#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXMESSAGE_LIB)
#  define NXMESSAGE_EXPORT Q_DECL_EXPORT
# else
#  define NXMESSAGE_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXMESSAGE_EXPORT
#endif


#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXENTITY_LIB)
#  define NXENTITY_EXPORT Q_DECL_EXPORT
# else
#  define NXENTITY_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXENTITY_EXPORT
#endif


#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(NXSPIKESCOPE_LIB)
#  define NX_SPIKE_SCOPE_EXPORT Q_DECL_EXPORT
# else
#  define NX_SPIKE_SCOPE_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_SPIKE_SCOPE_EXPORT
#endif

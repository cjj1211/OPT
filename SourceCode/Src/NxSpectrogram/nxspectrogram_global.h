#pragma once

#include <QtCore/qglobal.h>
#include <CLogger.h>

using namespace NX;

#ifndef BUILD_STATIC
# if defined(NXSPECTROGRAM_LIB)
#  define NX_SPECTROGRAM_EXPORT Q_DECL_EXPORT
# else
#  define NX_SPECTROGRAM_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_SPECTROGRAM_EXPORT
#endif

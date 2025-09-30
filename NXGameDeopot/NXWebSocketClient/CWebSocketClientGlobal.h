#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(WebSocketClient_LIB)
#  define NX_WebSocketClient_EXPORT __declspec(dllexport)
# else
#  define NX_WebSocketClient_EXPORT __declspec(dllimport)
# endif
#else
# define NX_WebSocketClient_EXPORT
#endif

#define BEGIN_NX_NAMESPACE namespace NX {
#define END_NX_NAMESPACE }


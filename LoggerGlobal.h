#pragma once

#ifndef BUILD_STATIC
# if defined(LOGGER_LIB)
#  define NX_LOGGER_EXPORT __declspec(dllexport)
# else
#  define NX_LOGGER_EXPORT __declspec(dllimport)
# endif
#else
# define NX_LOGGER_EXPORT
#endif

#define BEGIN_NX_NAMESPACE namespace NX {
#define END_NX_NAMESPACE }

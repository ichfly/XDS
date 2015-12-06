#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform definitions
/// Enumeration for defining the supported platforms
#define PLATFORM_NULL 0
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 2

////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform detection
#ifndef EMU_PLATFORM
#if defined( __WIN32__ ) || defined( _WIN32 )
#define EMU_PLATFORM PLATFORM_WINDOWS
#elif defined(__linux__)
#define EMU_PLATFORM PLATFORM_LINUX
#else
#error unsupported platform
#endif
#endif
#if defined(__x86_64__) || defined(_M_X64) || defined(__alpha__) || defined(__ia64__)
#define EMU_ARCHITECTURE_X64
#else
#define EMU_ARCHITECTURE_X86
#endif


#ifndef __func__
#define __func__ __FUNCTION__
#endif

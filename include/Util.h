#pragma once

#include "util/Mutex.h"
#include "util/Common.h"
#include "util/LowPath.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#define strcpy_s(a,b,c) strncpy(a,c,b)

#ifdef _WIN32

#ifndef snprintf
#define snprintf sprintf_s
#endif

#ifndef fseek64
#define fseek64 _fseeki64
#endif

#ifndef ftell64
#define ftell64 _ftelli64
#endif

#ifndef ftruncate
#define ftruncate _chsize_s
#endif

#define open _open
#define close _close
#define fileno _fileno

#define O_EXCL    _O_EXCL
#define O_WRONLY  _O_WRONLY

#else
#ifndef fseek64
#define fseek64 fseeko
#endif

#ifndef ftell64
#define ftell64 ftello
#endif
#endif
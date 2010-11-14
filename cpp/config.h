#pragma once

#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>

#include "debug.h"

typedef unsigned int uint;

#if DEBUG
static const bool kCheckAsserts = true;
#else
static const bool kCheckAsserts = false;
#endif

static const uint kMaxBoardSize = 19;

extern "C" {
#include "SFMT.h"
}

#ifdef _MSC_VER
#define _WINCON_ //don't want win32sdk COORD
#include <windows.h>
#include <process.h>
inline uint32_t millisTime() {
    return timeGetTime();
}
inline void reallyQuit() {
    TerminateProcess(GetCurrentProcess(), 0);
}
#else
#include <sys/time.h>
inline uint32_t millisTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#endif

#include "mutex.h"
#include "nat.h"
#include "natmap.h"
#include "natset.h"

#include "point.h"
#include "boardstate.h"
#include "pattern.h"
#include "chaininfo.h"
#include "board.h"
#include "gtp.h"

static std::string strprintf(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    return std::string(buf);
}


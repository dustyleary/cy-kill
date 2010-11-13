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
#include <windows.h>
inline uint32_t millisTime() {
    return timeGetTime();
}
#else
#include <sys/time.h>
inline uint32_t millisTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}
#endif

#include "nat.h"
#include "natmap.h"
#include "natset.h"

#include "point.h"
#include "boardstate.h"
#include "board.h"
#include "gtp.h"


#pragma once

#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <exception>

#include "debug.h"

typedef unsigned int uint;

#if DEBUG
static const bool kCheckAsserts = true;
#else
static const bool kCheckAsserts = false;
#endif

#include "nat.h"
#include "natmap.h"
#include "natset.h"

#include "intset.h"
#include "board.h"

extern "C" {
#include "SFMT.h"
}

#ifdef _MSC_VER
#include <windows.h>
inline uint32_t millisTime() {
    return timeGetTime();
}
#endif


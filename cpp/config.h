#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <algorithm>

#include "debug.h"

typedef unsigned int uint;
static const uint kBoardSize = 9;

#if DEBUG
static const bool kCheckAsserts = true;
#else
static const bool kCheckAsserts = false;
#endif

#include "nat.h"
#include "natmap.h"
#include "natset.h"

#include "intset.h"


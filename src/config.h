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
#include <set>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "debug.h"

typedef unsigned int uint;

#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_queue.h"
#include "tbb/task.h"
#include "tbb/atomic.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

//#define CYKILL_MT

#if DEBUG
static const bool kCheckAsserts = true;
#else
static const bool kCheckAsserts = false;
#endif

static const uint kMaxBoardSize = 19;

extern "C" {
#include "SFMT.h"
#include "sqlite3.h"
}

#ifdef _MSC_VER
#define _WINCON_ //don't want win32sdk COORD
#include <windows.h>
#include <process.h>
inline uint32_t cykill_millisTime() {
    return timeGetTime();
}
inline void cykill_quit(int code=0) {
    ExitProcess(code);
}
inline void cykill_sleep(uint millis) {
    return Sleep(millis);
}
inline void cykill_startthread(void (*func)(void*), void* data) {
    _beginthread(func, 0, data);
}
#else
#include <sys/time.h>
inline uint64_t cykill_microTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return uint64_t(tv.tv_sec)*1000000 + uint64_t(tv.tv_usec);
}
inline uint32_t cykill_millisTime() { return uint32_t(cykill_microTime() / 1000); }
inline void cykill_quit(int code=0) {
    exit(code);
}
inline void cykill_sleep(uint millis) {
    usleep(millis*1000);
}
inline void cykill_startthread(void (*func)(void*), void* data) {
    pthread_t thread;
    pthread_create(&thread, NULL, (void* (*)(void*)) func, data);
}
#endif

std::string trim(const std::string& in);
bool is_integer(const std::string& token);
bool is_double(const std::string& token);
int parse_integer(const std::string& token);
double parse_double(const std::string& token);

static std::string strprintf(const char* fmt, ...) {
    char buf[65536 * 4];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    return std::string(buf);
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#include "Nat.h"
#include "NatMap.h"
#include "NatSet.h"

#include "Point.h"
#include "PointColor.h"
#include "Zobrist.h"
#include "Pattern.h"
#include "ChainInfo.h"

#include "WeightedRandomChooser.h"

#include "TwoPlayerGridGame.h"

#include "Board.h"

#include "RandomPlayer.h"

#include "mcts2.h"
#include "OpeningBook.h"
#include "MysqlOpeningBook.h"

#include "Gtp.h"
#include "GtpMcts.h"
#include "GtpCyKill.h"

#include "TicTacToeGame.h"
#include "Connect4Game.h"

struct GlobalInitializer {
    GlobalInitializer() {
        init_gen_rand(cykill_millisTime());
        Zobrist::init();
    }
};

extern GlobalInitializer gGlobalInitializer;


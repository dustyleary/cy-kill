#pragma once

#include <stdarg.h>

void LOG(const char* _fmt, ...) {
    //force newline on the end
    char fmt[1024];
    strncpy(fmt, _fmt, 1023);
    char* fe = fmt + strlen(fmt);
    while(*fe == '\n' || *fe == ' ') {
        --fe;
    }
    *fe++ = '\n';
    *fe++ = 0;

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fflush(stdout);
    va_end(ap);
}

void afail(const char* msg, const char* file, int line, const char* func) {
    fprintf(stderr, "FAIL: %s %s %d %s\n", msg, file, line, func);
    fflush(stderr);
    exit(1);
}

#define AFAIL(msg) (afail(msg, __FILE__, __LINE__, __FUNCSIG__))
#define ASSERT(expr) { if(!(expr)) { AFAIL(#expr); } }


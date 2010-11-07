#pragma once

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
    char error[1024];
    sprintf(error, "ASSERT FAILED: (%s:%d %s): %s\n", file, line, func, msg);
    throw std::exception(error);
}

#define AFAIL(msg) (afail(msg, __FILE__, __LINE__, __FUNCSIG__))
#define ASSERT(expr) { if(kCheckAsserts && !(expr)) { AFAIL(#expr); } }
#undef assert


#include "config.h"

int main(int argc, char** argv) {
    if(argc!=3) {
        printf("USAGE: main-DumpPattern <size> <pattern>");
        return 1;
    }

    int size = atoi(argv[1]);

#define doit(N) \
    case N: { Pattern<N>::fromString(argv[2]).dump(); return 0; }
    
    switch(size) {
        doit(3)
        doit(5)
        doit(7)
        doit(9)
        doit(11)
        doit(13)
        doit(15)
        doit(17)
        doit(19)
    default:
        fprintf(stderr, "ERROR: unhandled size: %s\n", argv[1]);
        return 1;
    }
}



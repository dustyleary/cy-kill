#include "config.h"

int main(int argc, char** argv) {
    Gtp gtp(stdin, stdout, stderr);
    gtp.run();

    return 0;
}

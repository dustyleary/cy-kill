#include "config.h"

int main(int argc, char** argv) {
    cykill_init_globals();

    Gtp gtp(stdin, stdout, stderr);
    gtp.run();

    return 0;
}

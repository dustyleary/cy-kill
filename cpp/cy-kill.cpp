#include "config.h"

int main(int argc, char** argv) {
    int seed = millisTime();
    init_gen_rand(seed);

    Gtp gtp(stdin, stdout, stderr);

    gtp.run();

    return 0;
}

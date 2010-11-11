#include "config.h"

int main(int argc, char** argv) {
    int seed = millisTime();
    init_gen_rand(seed);

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Gtp gtp;

    while(true) {
        std::string line;
        if(!std::getline(std::cin, line)) {
            std::cout << "? error reading input\n\n";
            return 1;
        }
        std::cout << gtp.run_cmd(line) << std::flush;
    }

    return 0;
}

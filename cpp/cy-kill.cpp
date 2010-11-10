#include "config.h"

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    Gtp gtp;

    while(true) {
        std::string line;
        if(!std::getline(std::cin, line)) {
            std::cout << "? error reading input\n\n";
            return 1;
        }
        std::cout << gtp.run_cmd(line);
    }

    return 0;
}

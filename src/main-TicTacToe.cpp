#include "config.h"

class GtpTicTacToe : public Gtp {
};

int main(int argc, char** argv) {
    GtpTicTacToe gtp;
    gtp.run(argc, argv);

    return 0;
}


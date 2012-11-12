#include "config.h"

class GtpTicTacToe : public GtpMcts<TicTacToeGame> {
};

int main(int argc, char** argv) {
    GtpTicTacToe gtp;
    gtp.run(argc, argv);

    return 0;
}


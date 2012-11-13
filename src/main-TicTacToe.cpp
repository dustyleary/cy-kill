#include "config.h"

class GtpTicTacToe : public GtpMcts<TicTacToeGame> {
public:
    GtpTicTacToe() {
        uct_kRaveEquivalentPlayouts = 0;
        uct_kCountdownToCertainty = 1000000;
    }
};

int main(int argc, char** argv) {
    GtpTicTacToe gtp;
    gtp.run(argc, argv);

    return 0;
}


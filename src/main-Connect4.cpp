#include "config.h"

class GtpConnect4 : public GtpMcts<Connect4Game> {
public:
    GtpConnect4() {
        uct_kRaveEquivalentPlayouts = 0.001;
        uct_kCountdownToCertainty = 100000;
        uct_kMinVisitsForCertainty = 10000;
    }
};

int main(int argc, char** argv) {
    GtpConnect4 gtp;
    gtp.run(argc, argv);

    return 0;
}


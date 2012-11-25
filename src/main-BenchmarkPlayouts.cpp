#include "config.h"

template<typename GAME>
int benchmarkPlayouts(int playouts, bool usePureRandomPlayer) {
    printf("playouts: %d\n", playouts);

    fflush(stdout);
    PlayoutResults r;
    GAME board;
    board.setGammaPlayer(!usePureRandomPlayer);

    doRandomPlayouts<GAME>(board, playouts, PointColor::BLACK(), r);
    float dt = r.millis_taken / 1000.f;
    printf("\n");
    printf("total time: %.2f kpps: %.4f\n", dt, float(playouts)/(1000 * dt));
    printf("moves per game: %.2f\n", float(r.total_moves) / float(playouts));
    printf("%d/%d (black wins / white wins)\n", r.black_wins, r.white_wins);

    return 0;
}

int showUsage() {
    printf("USAGE: main-BenchmarkPlayouts <Go / TicTacToe> <PureRandomPlayer / GammaPlayer> [num_playouts]\n\n");
    return 1;
}

int main(int argc, char** argv) {
    if(argc<3 || argc>4) {
        return showUsage();
    }

    if(strcmp("PureRandomPlayer", argv[2]) && strcmp("GammaPlayer", argv[2])) {
        return showUsage();
    }

    int playouts = 1000;
    if(argc > 3) {
        playouts = atoi(argv[3]);
    }

    std::string gameName = argv[1];
    std::string randomPlayerName = argv[2];

    if(false);
    else if(gameName == "Go") return benchmarkPlayouts<Board>(playouts, randomPlayerName == "PureRandomPlayer");
    else if(gameName == "TicTacToe") return benchmarkPlayouts<TicTacToeGame>(playouts, randomPlayerName == "PureRandomPlayer");
    else {
        printf("ERROR: invalid gameName '%s'\n", gameName.c_str());
        printf("USAGE: main-BenchmarkPlayouts <Go / TicTacToe> <PureRandomPlayer / GammaPlayer> [num_playouts]\n\n");
        return 1;
    }
}


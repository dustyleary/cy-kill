#include "config.h"

int main(int argc, char** argv) {
    if(argc<2) {
        printf("USAGE: benchmark_playouts (PureRandomPlayer/GammaPlayer) [num_playouts]\n\n");
        return 1;
    }

    int playouts = 1;
    if(argc > 2) {
        playouts = atoi(argv[2]);
    }

    printf("playouts: %d\n", playouts);

    fflush(stdout);
    PlayoutResults r;
    Board board(19, 6.5);

    RandomPlayerPtr p = newRandomPlayer(argv[1]);
    p->doPlayouts(board, playouts, PointColor::BLACK(), r);
    float dt = r.millis_taken / 1000.f;
    printf("\n");
    printf("total time: %.2f kpps: %.4f\n", dt, float(playouts)/(1000 * dt));
    printf("moves per game: %.2f\n", float(r.total_moves) / float(playouts));
    printf("%d/%d (black wins / white wins)\n", r.black_wins, r.white_wins);

    return 0;
}


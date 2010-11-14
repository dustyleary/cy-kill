#include "config.h"

int main(int argc, char** argv) {
    cykill_init_globals();

    int playouts = 1;
    if(argc > 1) {
        playouts = atoi(argv[1]);
    }

    printf("playouts: %d\n", playouts);

    fflush(stdout);
    PlayoutResults r;
    Board board(19);

    RandomPlayer p;
    p.doPlayouts(board, playouts, 6.5f, BoardState::BLACK(), r);
    float dt = r.millis_taken / 1000.f;
    printf("\n");
    printf("total time: %.2f kpps: %.4f\n", dt, float(playouts)/(1000 * dt));
    printf("moves per game: %.2f\n", float(r.total_moves) / float(playouts));
    printf("%d/%d (black wins / white wins)\n", r.black_wins, r.white_wins);

    return 0;
}

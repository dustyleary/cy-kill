#include "config.h"

typedef Board<19> BOARD;
BOARD b;

uint total_moves = 0;
uint black_wins = 0;
uint white_wins = 0;

void playout() {
    b.reset();
    BoardState c = BoardState::WHITE();
    int passes = 0;
    int kos = 0;
    while(true) {
        c = c.enemy();
        b.playRandomMove(c);
        total_moves++;

        if(b.lastMove == BOARD::Point::pass()) {
            passes += 1;
            if(passes>=2) {
                break;
            }
        } else {
            passes = 0;
        }

        if(b.lastMoveWasKo()) {
            kos++;
            if(kos > 10) {
                break;
            }
        } else {
            kos = 0;
        }
    }
    if(b.trompTaylorScore() < -6) {
        black_wins++;
    } else {
        white_wins++;
    }
}

int main(int argc, char** argv) {
    int playouts = 1;
    if(argc > 1) {
        playouts = atoi(argv[1]);
    }
    int seed = millisTime();
    if(argc > 2) {
        seed = atoi(argv[2]);
    }

    init_gen_rand(seed);
    printf("playouts: %d\n", playouts);
    printf("    seed: %d\n", seed);

    fflush(stdout);
    uint32_t st = millisTime();
    for(int i=0; i<playouts; i++) {
        playout();
    }
    uint32_t et = millisTime();
    float dt = float(et-st) / 1000.f;
    printf("total time: %.2f kpps: %.4f\n", dt, float(playouts)/(1000 * dt));
    printf("moves per game: %.2f\n", float(total_moves) / float(playouts));
    printf("%d/%d (black wins / white wins)\n", black_wins, white_wins);

    b.dump();
    return 0;
}

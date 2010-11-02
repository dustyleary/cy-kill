#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "board.h"

extern "C" {
#include "SFMT.h"
}

typedef Board<3> BOARD;
BOARD b;

void playout() {
    b.reset();
    BoardState c = WHITE;
    BOARD::PointSet moves;
    int passes = 0;
    while(true) {
        b.dump();
        c = ENEMY(c);
        b.mcgMoves(c, moves);
        if(moves.size() == 0) {
            passes += 1;
            if(passes>=2) {
                break;
            }
            continue;
        }
        passes = 0;
        uint32_t mi = gen_rand32() % moves.size();
        b.makeMoveAssumeLegal(c, moves._list[mi]);
    }
}

int main(int argc, char** argv) {
    int playouts = 1;
    if(argc > 1) {
        playouts = atoi(argv[1]);
    }
    int seed = time(NULL);
    if(argc > 2) {
        seed = atoi(argv[2]);
    }
    init_gen_rand(seed);
    printf("playouts: %d\n", playouts);
    printf("    seed: %d\n", seed);

    for(int i=0; i<playouts; i++) {
        playout();
    }

    b.dump();
    return 0;
}

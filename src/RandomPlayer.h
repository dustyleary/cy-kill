#pragma once

template<typename GAME>
void doRandomPlayouts(const GAME& b, uint num_playouts, PointColor player_color, PlayoutResults& r) {
    ASSERT(!b.isGameFinished());

    uint32_t st = cykill_millisTime();
    GAME playout_board = b;

    for(uint i=0; i<num_playouts; i++) {
        playout_board = b;
        doRandomPlayout(playout_board, player_color, r);
    }
    uint32_t et = cykill_millisTime();
    r.millis_taken = et-st;
}

template<typename GAME>
void doRandomPlayout(GAME& playout_board, PointColor player_color, PlayoutResults& r) {
    ASSERT(!playout_board.isGameFinished());

    int passes = 0;
    int kos = 0;
    while(true) {
        //Move m = playRandomMove(playout_board, player_color);
        typename GAME::Move m = playout_board.getRandomMove(player_color);
        playout_board.playMoveAssumeLegal(m);
        //playout_board.dump();

        r.total_moves++;
        if(playout_board.isGameFinished()) { break; }
        player_color = player_color.enemy();
    }
    //LOG("DONE");
    PointColor winner = playout_board.winner();
    r.games++;
    if(winner == PointColor::WHITE()) {
        r.white_wins++;
    } else if(winner == PointColor::BLACK()) {
        r.black_wins++;
    }
}


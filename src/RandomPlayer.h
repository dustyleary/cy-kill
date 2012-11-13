#pragma once

template<typename GAME>
void doRandomPlayouts(const GAME& b, uint num_playouts, PointColor player_color, PlayoutResults& r) {
    ASSERT(!b.isGameFinished());

    uint32_t st = cykill_millisTime();
    GAME playout_board = b;

    for(uint i=0; i<num_playouts; i++) {
        playout_board = b;

        int passes = 0;
        int kos = 0;
        while(true) {
            //Move m = playRandomMove(playout_board, player_color);
            Move m = playout_board.getRandomMove(player_color);
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
    uint32_t et = cykill_millisTime();
    r.millis_taken = et-st;
}

//struct RandomPlayerBase {
//    typedef typename boost::shared_ptr<RandomPlayerBase> Ptr;
//
//    void doPlayouts(const GAME& b, uint num_playouts, PointColor player_color, PlayoutResults& r) {
//        uint32_t st = cykill_millisTime();
//        GAME playout_board = b;
//
//        for(uint i=0; i<num_playouts; i++) {
//            playout_board = b;
//            resetStateForNewBoard(playout_board);
//
//            int passes = 0;
//            int kos = 0;
//            while(true) {
//                Move m = playRandomMove(playout_board, player_color);
//
//                r.total_moves++;
//                if(playout_board.isGameFinished()) { break; }
//                player_color = player_color.enemy();
//            }
//            //LOG("DONE");
//            PointColor winner = playout_board.winner();
//            if(winner == PointColor::WHITE()) {
//                r.white_wins++;
//            } else if(winner == PointColor::BLACK()) {
//                r.black_wins++;
//            }
//        }
//        uint32_t et = cykill_millisTime();
//        r.millis_taken = et-st;
//    }
//    virtual void resetStateForNewBoard(GAME& b) {}
//    virtual ~RandomPlayerBase() =0;
//    virtual Move getRandomMove(GAME& b, PointColor c) =0;
//    virtual void movePlayed(GAME& b, Move m) {}
//
//    Move playRandomMove(GAME& b, PointColor c) {
//        //b.dump();
//        Move m = getRandomMove(b, c);
//        //LOG("%c %s", c.stateChar(), p.toGtpVertex(b.getSize()).c_str());
//        playMove(b, m);
//        return m;
//    }
//
//    void playMove(GAME& b, Move m) {
//        b.playMoveAssumeLegal(m);
//        movePlayed(b, m);
//    }
//};
//
//template<typename GAME>
//RandomPlayerBase<GAME>::~RandomPlayerBase() {}
//

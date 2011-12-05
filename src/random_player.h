#pragma once

struct RandomPlayerBase {
    void doPlayouts(const Board& b, uint num_playouts, BoardState player_color, PlayoutResults& r) {
        uint32_t st = cykill_millisTime();
        Board playout_board(b);

        for(uint i=0; i<num_playouts; i++) {
            memcpy(&playout_board, &b, sizeof(Board));
            resetStateForNewBoard(playout_board);

            int passes = 0;
            int kos = 0;
            while(true) {
                Move m = playRandomMove(playout_board, player_color);

                r.total_moves++;
                if(m.point == Point::pass()) {
                    passes++;
                    if(passes>=2) {
                        break;
                    }
                } else {
                    passes = 0;
                }
                if(playout_board.lastMoveWasKo()) {
                    kos++;
                    if(kos > 4) {
                        break;
                    }
                } else {
                    kos = 0;
                }
                player_color = player_color.enemy();
            }
            //LOG("DONE");
            double ttScore = playout_board.trompTaylorScore();
            if(ttScore > 0) {
                r.white_wins++;
            } else {
                r.black_wins++;
            }
        }
        uint32_t et = cykill_millisTime();
        r.millis_taken = et-st;
    }
    virtual void resetStateForNewBoard(Board& b) {}
    virtual ~RandomPlayerBase() =0;
    virtual Move getRandomMove(Board& b, BoardState c) =0;
    virtual void movePlayed(Board& b, Move m) {}

    Move playRandomMove(Board& b, BoardState c) {
        //b.dump();
        Move m = getRandomMove(b, c);
        //LOG("%c %s", c.stateChar(), p.toGtpVertex(b.getSize()).c_str());
        playMove(b, m);
        return m;
    }

    void playMove(Board& b, Move m) {
        b.playMoveAssumeLegal(m);
        movePlayed(b, m);
    }
};

typedef boost::shared_ptr<RandomPlayerBase> RandomPlayerPtr;
RandomPlayerPtr newRandomPlayer(const std::string& className);

struct PureRandomPlayer : public RandomPlayerBase {
    Move getRandomMove(Board& b, BoardState c) {
        if(!b.emptyPoints.size()) {
            return Move(c, Point::pass());
        }

        uint32_t mi = ::gen_rand64() % b.emptyPoints.size();
        uint32_t si = mi;
        while(true) {
            Point p = b.emptyPoints[mi];
            Move m(c, p);
            if(b.isValidMove(m) && !b.isSimpleEye(m)) {
                return m;
            }
            mi = (mi + 1) % b.emptyPoints.size();
            if(mi == si) {
                return Move(c, Point::pass());
            }
        }
    }

};


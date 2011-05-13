#pragma once

struct RandomPlayerBase {
    void doPlayouts(const Board& b, uint num_playouts, float komi, BoardState player_color, PlayoutResults& r) {
        uint32_t st = cykill_millisTime();
        Board playout_board(b);

        for(uint i=0; i<num_playouts; i++) {
            memcpy(&playout_board, &b, sizeof(Board));
            resetStateForNewBoard(playout_board);

            int passes = 0;
            int kos = 0;
            while(true) {
                Point p = playRandomMove(playout_board, player_color);

                r.total_moves++;
                if(p == Point::pass()) {
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
            int ttScore = playout_board.trompTaylorScore();
            if(komi + ttScore > 0) {
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
    virtual Point getRandomMove(Board& b, BoardState c) =0;

    Point playRandomMove(Board& b, BoardState c) {
        Point p = getRandomMove(b, c);
        b.playMoveAssumeLegal(c, p);
        return p;
    }
};

typedef boost::shared_ptr<RandomPlayerBase> RandomPlayerPtr;
RandomPlayerPtr newRandomPlayer(const std::string& className);

struct PureRandomPlayer : public RandomPlayerBase {
    Point getRandomMove(Board& b, BoardState c) {
        if(!b.emptyPoints.size()) {
            return Point::pass();
        }

        uint32_t mi = ::gen_rand32() % b.emptyPoints.size();
        uint32_t si = mi;
        while(true) {
            Point p = b.emptyPoints[mi];
            if(b.isValidMcgMove(c, p)) {
                return p;
            }
            mi = (mi + 1) % b.emptyPoints.size();
            if(mi == si) {
                return Point::pass();
            }
        }
    }

};


#pragma once

struct Gammas {
    double gammas[2<<20];
    void load(const char* filename);
    Gammas();
};
extern Gammas gGammas;

struct GammaPlayer {
    Point getRandomMove(Board& b, BoardState c) {
        if(!b.emptyPoints.size()) {
            return Point::pass();
        }

        PointSet valid_moves;
        NatMap<Point, Pat3> pats;

        memcpy(&valid_moves, &b.emptyPoints, sizeof(valid_moves));

        double weight_total = 0;
        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];
            if(!b.isValidMcgMove(c, p)) {
                valid_moves.remove(p);
                continue;
            }
            Pat3 pat = b.canonicalPatternAt<3>(c, p);
            pats[p] = pat;

            double pw = gGammas.gammas[pat.toUint()];
            weight_total += pw;
        }
        if(!valid_moves.size()) {
            return Point::pass();
        }

        double r = genrand_res53() * weight_total;

        Point p;
        for(uint i=0; i<valid_moves.size(); i++) {
            p = valid_moves[i];
            Pat3 pat = pats[p];
            double pw = gGammas.gammas[pat.toUint()];
            if(r<pw) {
                break;
            }
            r -= pw;
        }
        if(!p.isValid()) {
            LOG("BBB");
        }
        return p;
    }

    Point playRandomMove(Board& b, BoardState c) {
        Point p = getRandomMove(b, c);
        b.playMoveAssumeLegal(c, p);
        return p;
    }

    void doPlayouts(Board& b, uint num_playouts, float komi, BoardState player_color, PlayoutResults& r) {
        r.black_wins = 0;
        r.white_wins = 0;

        uint32_t st = cykill_millisTime();
        Board playout_board(b);

        int neg_komi = (int)(-floor(komi));

        for(uint i=0; i<num_playouts; i++) {
            memcpy(&playout_board, &b, sizeof(Board));

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
            if(playout_board.trompTaylorScore() < neg_komi) {
                r.black_wins++;
            } else {
                r.white_wins++;
            }
        }
        uint32_t et = cykill_millisTime();
        r.millis_taken = et-st;
    }
};


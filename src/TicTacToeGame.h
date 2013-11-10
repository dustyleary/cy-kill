#include "config.h"

class TicTacToeGame : public TwoPlayerGridGame {
public:
    TicTacToeGame() : TwoPlayerGridGame(3) {
        reset();
    }

    PointColor winner() const {
        //vertical
        for(int y=0; y<getSize(); y++) {
            PointColor c0 = bs(COORD(0,y));
            if(c0 == PointColor::EMPTY()) continue;
            if(c0 != bs(COORD(1,y))) continue;
            if(c0 != bs(COORD(2,y))) continue;
            return c0;
        }

        //horizontal
        for(int x=0; x<getSize(); x++) {
            PointColor c0 = bs(COORD(x,0));
            if(c0 == PointColor::EMPTY()) continue;
            if(c0 != bs(COORD(x,1))) continue;
            if(c0 != bs(COORD(x,2))) continue;
            return c0;
        }

        //diagonal
        PointColor c0 = bs(COORD(1,1));
        if(c0 == PointColor::EMPTY()) return c0;
        if(true && c0 == bs(COORD(0,0))
                && c0 == bs(COORD(2,2))) {
            return c0;
        }
        if(true && c0 == bs(COORD(0,2))
                && c0 == bs(COORD(2,0))) {
            return c0;
        }
        return PointColor::EMPTY();
    }

    void _playMoveAssumeLegal(Move m) {
        set_bs(m.point, m.color);
    }

    bool isGameFinished() const {
        if(winner() != PointColor::EMPTY()) return true;
        for(int x=0; x<getSize(); x++) {
            for(int y=0; y<getSize(); y++) {
                if(PointColor::EMPTY() == bs(COORD(x,y))) return false;
            }
        }
        return true;
    }

    bool isValidMove(Move m) const { return isValidMove(m.color, m.point); }
    bool isValidMove(PointColor color, Point p) const {
        if(!isOnBoard(p)) return false;
        if(bs(p) != PointColor::EMPTY()) return false;
        return true;
    }

    uint64_t zobrist() const {
        uint64_t r = computeBoardHash();

        //whos turn it is
        Point turnPoint = COORD(5,-1);
        if(getWhosTurn() == PointColor::BLACK()) {
          r ^= Zobrist::black[turnPoint];
        } else {
          r ^= Zobrist::white[turnPoint];
        }
        return r;
    }

    void getValidMoves(PointColor c, std::vector<Move>& out, uint moduloNumerator=0, uint moduloDenominator=1) const {
        out.clear();
        out.reserve(getSize() * getSize());
        FOREACH_BOARD_POINT(p, {
            if(isValidMove(c, p)) {
                out.push_back(Move(c, p));
            }
        });
    }
    void getCanonicalValidMoves(PointColor c, std::vector<Move>& out) {
        getValidMoves(c, out);
    }

    Move getRandomMove(PointColor c) {
        int seen = 0;
        Move current(c, Point::pass());
        FOREACH_BOARD_POINT(p, {
            if(bs(p) == PointColor::EMPTY()) {
                ++seen;
                double chance = 1.0 / seen;
                if(genrand_res53() < chance) {
                    current = Move(c, p);
                }
            }
        });
        return current;
    }

    void setGammaPlayer(bool b) {}
};


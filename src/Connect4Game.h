#include "config.h"

class Connect4Game : public TwoPlayerGridGame {
    PointColor mWinner;
    uint mHeight;

public:
    Connect4Game(uint w=7, uint h=6) : TwoPlayerGridGame(w), mHeight(h) {
        ASSERT(h<=w);
        reset();
    }

    void reset() {
        TwoPlayerGridGame::reset();
        mWinner = PointColor::EMPTY();
    }

    void _playMoveAssumeLegal(Move m) {
        ASSERT(PointColor::EMPTY() == bs(m.point));
        while(m.point.y() < mHeight-1 && PointColor::EMPTY() == bs(COORD(m.point.x(), m.point.y()+1))) {
          m.point = COORD(m.point.x(), m.point.y()+1);
        }
        set_bs(m.point, m.color);
        mWinner = findWinner(m);
    }

    PointColor findWinner(Move m) {
        //find winner horizontal
        int left_x = m.point.x();
        while(left_x>0 && bs(COORD(left_x-1, m.point.y())) == PointColor::EMPTY()) {
          left_x--;
        }
        if(true && m.color == bs(COORD(left_x+1, m.point.y()))
                && m.color == bs(COORD(left_x+2, m.point.y()))
                && m.color == bs(COORD(left_x+3, m.point.y()))) {
          return m.color;
        }

        //find winner vertical
        if(true && m.color == bs(COORD(m.point.x(), m.point.y()+1))
                && m.color == bs(COORD(m.point.x(), m.point.y()+2))
                && m.color == bs(COORD(m.point.x(), m.point.y()+3))) {
          return m.color;
        }

        //find winner diag
        if(true && m.color == bs(COORD(m.point.x()+1, m.point.y()+1))
                && m.color == bs(COORD(m.point.x()+2, m.point.y()+2))
                && m.color == bs(COORD(m.point.x()+3, m.point.y()+3))) {
          return m.color;
        }
        if(true && m.color == bs(COORD(m.point.x()+1, m.point.y()-1))
                && m.color == bs(COORD(m.point.x()+2, m.point.y()-2))
                && m.color == bs(COORD(m.point.x()+3, m.point.y()-3))) {
          return m.color;
        }
        if(true && m.color == bs(COORD(m.point.x()-1, m.point.y()+1))
                && m.color == bs(COORD(m.point.x()-2, m.point.y()+2))
                && m.color == bs(COORD(m.point.x()-3, m.point.y()+3))) {
          return m.color;
        }
        if(true && m.color == bs(COORD(m.point.x()-1, m.point.y()-1))
                && m.color == bs(COORD(m.point.x()-2, m.point.y()-2))
                && m.color == bs(COORD(m.point.x()-3, m.point.y()-3))) {
          return m.color;
        }
        return PointColor::EMPTY();
    }

    PointColor winner() const { return mWinner; }

    bool isGameFinished() const {
        if(winner() != PointColor::EMPTY()) return true;
        for(int x=0; x<getSize(); x++) {
            for(int y=0; y<mHeight; y++) {
                if(PointColor::EMPTY() == bs(COORD(x,y))) return false;
            }
        }
        return true;
    }

    bool isValidMove(Move m) const { return isValidMove(m.color, m.point); }
    bool isValidMove(PointColor color, Point p) const {
        if(p.y() != 0) return false;
        if(p.x() >= getSize()) return false;
        if(bs(p) != PointColor::EMPTY()) return false;
        return true;
    }

    uint64_t zobrist() const {
        uint64_t r = boardHash();

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
        out.reserve(getSize());
        for(int x=0; x<getSize(); x++) {
            Point p = COORD(x,0);
            if(PointColor::EMPTY() == bs(p)) {
                out.push_back(Move(c, p));
            }
        }
    }

    Move getRandomMove(PointColor c) {
        int seen = 0;
        Move current(c, Point::pass());
        for(int x=0; x<getSize(); x++) {
            Point p = COORD(x,0);
            if(isValidMove(c, p)) {
                ++seen;
                double chance = 1.0 / seen;
                if(genrand_res53() < chance) {
                    current = Move(c, p);
                }
            }
        }
        return current;
    }

    Move getGammaMove(PointColor c) {
        return getRandomMove(c);
    }
};


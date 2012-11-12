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
        set_bs(m.point, m.playerColor);
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
};


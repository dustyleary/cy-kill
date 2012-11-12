#include "config.h"

class TicTacToeGame {
public:
    struct Move {
      PointColor playerColor;
      Point point;
      Move() : playerColor(PointColor::EMPTY()), point(Point::invalid()) {
      }
      Move(PointColor c, Point p) : playerColor(c), point(p) {
        ASSERT(c.isPlayer());
      }
      Move(PointColor c, int x, int y) : playerColor(c), point(COORD(x,y)) {
        ASSERT(c.isPlayer());
      }
      Move& operator=(const Move& r) {
        playerColor = r.playerColor;
        point = r.point;
        return *this;
      }
      bool operator==(const Move& r) const {
        return (playerColor == r.playerColor) && (point == r.point);
      }
      bool operator!=(const Move& r) const { return !operator==(r); }
      bool operator<(const Move& r) const {
        if(playerColor.toUint() < r.playerColor.toUint()) return true;
        if(playerColor.toUint() > r.playerColor.toUint()) return false;
        return point.toUint() < r.point.toUint();
      }
    };

    NatMap<Point, PointColor> states;
    void set_bs(Point p, PointColor c) { states[p] = c; }
    const PointColor& bs(Point p) const {
#if 0
        fprintf(stderr, "bs(%d,%d) = '%c'\n", p.x(), p.y(), states[p].stateChar());
#endif
        return states[p];
    }

public:
    TicTacToeGame() {
        reset();
    }

    uint getSize() const {
        return 3;
    }

    void reset() {
        for(int y=-1; y<=(int)getSize(); y++) {
            set_bs(COORD(-1, y), PointColor::WALL());
            set_bs(COORD(getSize(), y), PointColor::WALL());
        }
        for(int x=-1; x<=(int)getSize(); x++) {
            set_bs(COORD(x, -1), PointColor::WALL());
            set_bs(COORD(x, getSize()), PointColor::WALL());
        }
        for(int x=0; x<getSize(); x++) {
            for(int y=0; y<getSize(); y++) {
                set_bs(COORD(x,y), PointColor::EMPTY());
            }
        }
    }

    void dump() const {
        for(int y=-1; y<=(int)getSize(); y++) {
            for(int x=-1; x<=(int)getSize(); x++) {
                Point p = COORD(x,y);
                putc(bs(p).stateChar(), stderr);
            }
            putc('\n', stderr);
        }
        fflush(stderr);
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

    void playMoveAssumeLegal(Move m) {
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


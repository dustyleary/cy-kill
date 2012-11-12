#pragma once

struct TwoPlayerGridGame {
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

    uint size;
    Move lastMove;

    NatMap<Point, PointColor> states;

#define FOREACH_BOARD_POINT(p, block) \
    for(uint y=0; y<getSize(); y++) { \
        for(uint x=0; x<getSize(); x++) { \
            Point p = COORD(x,y); \
            block \
        } \
    }

    TwoPlayerGridGame(uint s) : size(s) {
        reset();
    }

    //TwoPlayerGridGame(const TwoPlayerGridGame& other) {
    //    memcpy(this, &other, sizeof(TwoPlayerGridGame));
    //}

    //TwoPlayerGridGame& operator=(const TwoPlayerGridGame& other) {
    //    memcpy(this, &other, sizeof(TwoPlayerGridGame));
    //    return *this;
    //}

    void set_bs(Point p, PointColor c) { states[p] = c; }
    const PointColor& bs(Point p) const {
#if 0
        fprintf(stderr, "bs(%d,%d) = '%c'\n", p.x(), p.y(), states[p].stateChar());
#endif
        return states[p];
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
        FOREACH_BOARD_POINT(p, {
            set_bs(p, PointColor::EMPTY());
        });
    }

    uint getSize() const {
        return size;
    }

    PointColor getWhosTurn() const {
      if(lastMove.point != Point::invalid()) {
        return lastMove.playerColor.enemy();
      } else {
        return PointColor::BLACK();
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

    bool isOnBoard(Point p) const {
        return p.x() < getSize() && p.y() < getSize();
    }

    virtual void _playMoveAssumeLegal(Move m) =0;
    virtual void assertGoodState() {}

    void playMoveAssumeLegal(Move m) {
        _playMoveAssumeLegal(m);

        lastMove = m;

        //dump();
        assertGoodState();
    }

};


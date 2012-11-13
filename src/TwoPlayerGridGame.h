#pragma once

struct PlayoutResults {
    uint games;
    uint black_wins;
    uint white_wins;
    uint total_moves;
    uint millis_taken;
    PlayoutResults()
        : games(0)
        , black_wins(0)
        , white_wins(0)
        , total_moves(0)
        , millis_taken(0)
    {
    }
};

struct TwoPlayerGridGame {
    struct Move {
      PointColor color;
      Point point;
      Move() : color(PointColor::EMPTY()), point(Point::invalid()) {
      }
      Move(PointColor c, Point p) : color(c), point(p) {
        ASSERT(c.isPlayer());
      }
      Move(PointColor c, int x, int y) : color(c), point(COORD(x,y)) {
        ASSERT(c.isPlayer());
      }
      Move& operator=(const Move& r) {
        color = r.color;
        point = r.point;
        return *this;
      }
      bool operator==(const Move& r) const {
        return (color == r.color) && (point == r.point);
      }
      bool operator!=(const Move& r) const { return !operator==(r); }
      bool operator<(const Move& r) const {
        if(color.toUint() < r.color.toUint()) return true;
        if(color.toUint() > r.color.toUint()) return false;
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
        return lastMove.color.enemy();
      } else {
        return PointColor::BLACK();
      }
    }

    uint64_t boardHash() const {
        uint64_t r = Zobrist::black[Point::pass()];
        FOREACH_BOARD_POINT(p, {
            if(bs(p) == PointColor::BLACK()) {
                r ^= Zobrist::black[p];
            } else if(bs(p) == PointColor::WHITE()) {
                r ^= Zobrist::white[p];
            }
        });
        return r;
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


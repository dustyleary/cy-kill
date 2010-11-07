#pragma once

#include "config.h"

static const char* BOARD_STATE_CHARS = ".XO#";
struct BoardState : public Nat<BoardState> {
    PRIVATE_NAT_CONSTRUCTOR(BoardState);
    static const uint kBound = 4;

    BoardState() : Nat<BoardState>(0) {}
    static BoardState EMPTY() { return BoardState(0); }
    static BoardState BLACK() { return BoardState(1); }
    static BoardState WHITE() { return BoardState(2); }
    static BoardState WALL()  { return BoardState(3); }

    BoardState enemy() {
        ASSERT(*this == BLACK() || *this == WHITE());
        return BoardState(toUint() ^ 3);
    }

    char stateChar() const { return BOARD_STATE_CHARS[toUint()]; }
};

#define POS(x, y) (((x+1)<<8) | (y+1))
#define X(p) ((p>>8)-1)
#define Y(p) ((p&0xff)-1)

#define N(p) (p-1)
#define S(p) (p+1)
#define E(p) (p+(1<<8))
#define W(p) (p-(1<<8))

typedef uint16_t Point;
typedef uint8_t ChainIndex;

template<uint kBoardSize>
struct Board {
    static const int kBoardArraySize = (kBoardSize+2)*(kBoardSize+1)+1;
    static const int kPlaySize = kBoardSize*kBoardSize;
    static const int kMaxChains = (kPlaySize*3)/4;

    static uint8_t getSize() { return kBoardSize; }
    static int offset(Point p) { return (X(p)+1)+(Y(p)+1)*(kBoardSize+1); }

    static Point NatMap(Point p) { return X(p)+Y(p)*kBoardSize; }

    typedef IntSet<Point, kPlaySize, Board> PointSet;

    struct Chain {
        PointSet stones;
        PointSet liberties;
        bool dead;
        void reset() {
            stones.reset();
            liberties.reset();
            dead = false;
        }
    };
    Chain chains[kMaxChains];

    BoardState states[kBoardArraySize];
    ChainIndex chain_indexes[kBoardArraySize];
    int chain_count;
    Point koPoint;
    PointSet emptyPoints;

    Board() {
        reset();
    }

    void reset() {
        chain_count = 0;
        koPoint = POS(-1,-1);
        for(int i=0; i<kBoardArraySize; i++) {
            states[i] = BoardState::EMPTY();
        }
        memset(chain_indexes, 0, sizeof(chain_indexes));
        for(int y=0; y<(kBoardSize+2); y++) {
            states[y*(kBoardSize+1)] = BoardState::WALL();
        }
        for(int x=0; x<(kBoardSize+2); x++) {
            states[x] = BoardState::WALL();
            states[(kBoardSize+1)*(kBoardSize+1) + x] = BoardState::WALL();
        }
        emptyPoints.reset();
        for(int x=0; x<kBoardSize; x++) {
            for(int y=0; y<kBoardSize; y++) {
                emptyPoints.add(POS(x,y));
            }
        }
    }

    void assertGoodState() {
        //walls are intact
        for(int y=0; y<(kBoardSize+2); y++) {
            ASSERT(states[y*(kBoardSize+1)] == BoardState::WALL());
        }
        for(int x=0; x<(kBoardSize+2); x++) {
            ASSERT(states[x] == BoardState::WALL());
            ASSERT(states[(kBoardSize+1)*(kBoardSize+1) + x] == BoardState::WALL());
        }
        //empty points are correct
        for(int y=0; y<kBoardSize; y++) {
            for(int x=0; x<kBoardSize; x++) {
                Point p = POS(x,y);
                if(bs(p) == BoardState::EMPTY()) {
                    ASSERT(emptyPoints.contains(p));
                } else {
                    ASSERT(!emptyPoints.contains(p));
                }
            }
        }
    }

    void dump() {
        for(int y=0; y<(kBoardSize+2); y++) {
            for(int x=0; x<(kBoardSize+2); x++) {
                putc(states[y*(kBoardSize+1)+x].stateChar(), stdout);
            }
            putc('\n', stdout);
        }
        fflush(stdout);
    }

    BoardState& bs(Point p) { return states[offset(p)]; }

    uint8_t countLiberties(Point p) {
        BoardState v = bs(p);
        if(v == BoardState::EMPTY()) return 0;
        int ci = chain_indexes[offset(p)]-1;
        Chain& c = chains[ci];
        return c.liberties.size();
    }

    void mergeChains(Point dest, Point inc) {
        int di = chain_indexes[offset(dest)]-1;
        int ii = chain_indexes[offset(inc)]-1;
        if(di == ii) return;
        Chain& cd = chains[di];
        Chain& ci = chains[ii];
        cd.liberties.addAll(ci.liberties);
        cd.stones.addAll(ci.stones);
        for(int i=0; i<ci.stones._size; i++) {
            chain_indexes[offset(ci.stones._list[i])] = di+1;
        }
    }

    void chainAddPoint(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.stones.add(p);

        chain_indexes[offset(p)] = ci+1;

#define doit(D) \
        if(bs(D(p)) == BoardState::EMPTY()) { c.liberties.add(D(p)); } \
        else if(bs(D(p)) == bs(p)) { mergeChains(p, D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        c.liberties.remove(p);
    }

    void chainAddLiberty(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.liberties.add(p);
    }

    void makeNewChain(Point p) {
        int ci;
        for(ci=0; ci<chain_count; ci++) {
            //try to find a chain index to reuse
            if(chains[ci].dead) {
                break;
            }
        }
        if(ci == chain_count) {
            chain_count++;
        }
        chain_indexes[offset(p)] = ci+1;

        Chain& c = chains[ci];
        c.reset();
        c.stones.add(p);
#define doit(D) if(bs(D(p)) == BoardState::EMPTY()) { c.liberties.add(D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
    }

    void chainRemoveLiberty(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.liberties.remove(p);
        if(c.liberties.size() == 0) {
            //kill
            for(int i=0; i<c.stones._size; i++) {
                Point p = c.stones._list[i];
                bs(p) = BoardState::EMPTY();
                emptyPoints.add(p);
                chain_indexes[offset(p)] = 0;
            }
            for(int i=0; i<c.stones._size; i++) {
                Point p = c.stones._list[i];
#define doit(D) if(bs(D(p)) == BoardState::BLACK() || bs(D(p)) == BoardState::WHITE()) { chainAddLiberty(D(p), p); }
                doit(N)
                doit(S)
                doit(E)
                doit(W)
#undef doit
            }
            if(c.stones._size == 1) {
                koPoint = c.stones._list[0];
            }
            c.dead = true;
        }
    }

    void makeMoveAssumeLegal(BoardState c, Point p) {
        koPoint = POS(-1, -1);

        bs(p) = c;
        if(false) {}
#define doit(D) else if(bs(D(p)) == c) { chainAddPoint(D(p), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        else {
            makeNewChain(p);
        }

        BoardState ec = c.enemy();
#define doit(D) if(bs(D(p)) == ec) { chainRemoveLiberty(D(p), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        emptyPoints.remove(p);
    }

    bool isSuicide(BoardState c, Point p) {
#define doit(D) if(bs(D(p)) == BoardState::EMPTY()) { return false; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        BoardState ec = c.enemy();
#define doit(D) if(bs(D(p)) == ec && (countLiberties(D(p)) == 1)) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

#define doit(D) if(bs(D(p)) == c && countLiberties(D(p)) > 1) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        return true;
    }

    bool isSimpleEye(BoardState c, Point p) {
        if(bs(p) != BoardState::EMPTY()) return false;
#define doit(D) if(bs(D(p)) != c && bs(D(p)) != BoardState::WALL()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        BoardState ec = c.enemy();
        int enemy_diagonal_count = 0;
#define doit(D,F) if(bs(D(F(p))) == ec) { enemy_diagonal_count++; }
        doit(N,E)
        doit(N,W)
        doit(S,E)
        doit(S,W)
#undef doit
        if(enemy_diagonal_count == 0) return true;
        if(enemy_diagonal_count >= 2) return false;

        int wall_count = 0;
#define doit(D) if(bs(D(p)) == BoardState::WALL()) { wall_count++; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        if(wall_count>1) return false;
        return true;
    }

    bool lastMoveWasKo() {
        return koPoint != POS(-1,-1);
    }

    void mcgMoves(BoardState c, PointSet& ps) {
        memcpy(&ps, &emptyPoints, sizeof(ps));
        if(koPoint != POS(-1,-1)) {
            ps.remove(koPoint);
        }
        for(int i=0; i<emptyPoints._size; i++) {
            Point p = emptyPoints._list[i];
            if(isSimpleEye(c,p) || isSuicide(c,p)) { ps.remove(p); }
        }
    }

    bool _lastMoveWasPass;
    bool lastMoveWasPass() { return _lastMoveWasPass; }

    void playRandomMove(BoardState c) {
        PointSet moves;
        mcgMoves(c, moves);
        if(moves.size() == 0) {
            _lastMoveWasPass = true;
            return;
        }
        _lastMoveWasPass = false;
        uint32_t mi = gen_rand32() % moves.size();
        b.makeMoveAssumeLegal(c, moves._list[mi]);
    }

};


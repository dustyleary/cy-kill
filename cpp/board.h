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
        void reset() {
            _stones.reset();
            _liberties.reset();
        }
        uint size() const { return _stones.size(); }
        bool isDead() const { return _liberties.size() == 0; }

        void addStone(Point p) { _stones.add(p); }
        void addLiberty(Point p) { _liberties.add(p); }
        void removeLiberty(Point p) { _liberties.remove(p); }
        void merge(Chain& o) {
            _liberties.addAll(o._liberties);
            _stones.addAll(o._stones);
        }
        bool isInAtari() const { return _liberties.size() == 1; }

        PointSet _stones;
        PointSet _liberties;
    };
    Chain chains[kMaxChains];

#define FOREACH_CHAIN_STONE(chain, pt, block) \
    { \
        for(uint i=0; i<chain.size(); i++) { \
            Point pt = chain._stones._list[i]; \
            block \
        } \
    }

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
    const BoardState& bs(Point p) const { return states[offset(p)]; }

    Chain& chainAt(Point p) {
        int ci = chain_indexes[offset(p)]-1;
        return chains[ci];
    }

    const Chain& chainAt(Point p) const {
        int ci = chain_indexes[offset(p)]-1;
        return chains[ci];
    }

    uint8_t countLiberties(Point p) const {
        if(bs(p) != BoardState::BLACK() && bs(p) != BoardState::WHITE()) return 0;
        uint8_t result = 0;
        for(uint y=0; y<kBoardSize; y++) {
            for(uint x=0; x<kBoardSize; x++) {
                Point lp = POS(x,y);
                if(bs(lp) != BoardState::EMPTY()) continue;
#define doit(D) if(chain_indexes[offset(D(lp))] == chain_indexes[offset(p)]) { result++; continue; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
            }
        }
        return result;
    }

    void mergeChains(Point dest, Point inc) {
        int di = chain_indexes[offset(dest)];
        int ii = chain_indexes[offset(inc)];
        if(di == ii) return;
        Chain& cd = chainAt(dest);
        Chain& ci = chainAt(inc);
        cd.merge(ci);
        FOREACH_CHAIN_STONE(ci, p, {
            chain_indexes[offset(p)] = di;
        });
    }

    void chainAddPoint(Point chain, Point p) {
        chainAt(chain).addStone(p);
        chain_indexes[offset(p)] = chain_indexes[offset(chain)];

#define doit(D) \
        if(bs(D(p)) == BoardState::EMPTY()) { chainAt(chain).addLiberty(D(p)); } \
        else if(bs(D(p)) == bs(p)) { mergeChains(chain, D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        chainAt(chain).removeLiberty(p);
    }

    void makeNewChain(Point p) {
        int ci;
        for(ci=0; ci<chain_count; ci++) {
            //try to find a chain index to reuse
            if(chains[ci].isDead()) {
                break;
            }
        }
        if(ci == chain_count) {
            chain_count++;
        }
        chain_indexes[offset(p)] = ci+1;

        Chain& c = chains[ci];
        c.reset();
        c.addStone(p);
#define doit(D) if(bs(D(p)) == BoardState::EMPTY()) { c.addLiberty(D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
    }

    void chainAddLiberty(Point chain, Point p) {
        chainAt(chain).addLiberty(p);
    }

    void chainRemoveLiberty(Point chain, Point p) {
        Chain& c = chainAt(chain);
        c.removeLiberty(p);
        if(c.isDead()) {
            //kill
            FOREACH_CHAIN_STONE(c, p, {
                bs(p) = BoardState::EMPTY();
                emptyPoints.add(p);
                chain_indexes[offset(p)] = 0;
            });
#define doit(D) if(bs(D(p)) == BoardState::BLACK() || bs(D(p)) == BoardState::WHITE()) { chainAddLiberty(D(p), p); }
            FOREACH_CHAIN_STONE(c, p, {
                doit(N)
                doit(S)
                doit(E)
                doit(W)
            });
#undef doit
            if(c.size() == 1) {
                koPoint = chain;
            }
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

    bool isSuicide(BoardState c, Point p) const {
#define doit(D) if(bs(D(p)) == BoardState::EMPTY()) { return false; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        BoardState ec = c.enemy();
#define doit(D) if(bs(D(p)) == ec && chainAt(D(p)).isInAtari()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

#define doit(D) if(bs(D(p)) == c && !chainAt(D(p)).isInAtari()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        return true;
    }

    bool isSimpleEye(BoardState c, Point p) const {
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

    bool isValidMove(BoardState c, Point p) const {
        if(bs(p) != BoardState::EMPTY()) return false;
        if(isSuicide(c, p)) return false;
        if(p == koPoint) return false;
        return true;
    }

    bool isValidMcgMove(BoardState c, Point p) const {
        return isValidMove(c, p) && !isSimpleEye(c,p);
    }

    void mcgMoves(BoardState c, PointSet& ps) {
        ps.reset();
        for(int y=0; y<kBoardSize; y++) {
            for(int x=0; x<kBoardSize; x++) {
                Point p = POS(x,y);
                if(isValidMcgMove(c, p)) {
                    ps.add(p);
                }
            }
        }
    }

    bool _lastMoveWasPass;
    bool lastMoveWasPass() { return _lastMoveWasPass; }

    void playRandomMove(BoardState c) {
        uint32_t mi = gen_rand32() % emptyPoints.size();
        uint32_t si = mi;
        while(true) {
            Point p = emptyPoints[mi];
            if(isValidMcgMove(c, p)) {
                _lastMoveWasPass = false;
                makeMoveAssumeLegal(c, p);
                break;
            }
            mi = (mi + 1) % emptyPoints.size();
            if(mi == si) {
                _lastMoveWasPass = true;
                break;
            }
        }
    }

};


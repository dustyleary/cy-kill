#pragma once

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

template<uint kBoardSize>
struct Point : public Nat<Point<kBoardSize> > {
    PRIVATE_NAT_CONSTRUCTOR(Point);
    static const uint kBound = (kBoardSize+2)*(kBoardSize+1)+1;

    static Point fromCoord(int x, int y) { return Point((x+1) + (y+1)*(kBoardSize+1)); }
    uint x() const { return toUint() % (kBoardSize+1) - 1; }
    uint y() const { return toUint() / (kBoardSize+1) - 1; }

    Point N() { return Point(toUint() - (kBoardSize+1)); }
    Point S() { return Point(toUint() + (kBoardSize+1)); }
    Point E() { return Point(toUint() + 1); }
    Point W() { return Point(toUint() - 1); }

    Point() : Nat(-1) {}
};

typedef uint8_t ChainIndex;

template<uint kBoardSize>
struct Board {
    typedef Point<kBoardSize> Point;

    static const int kBoardArraySize = (kBoardSize+2)*(kBoardSize+1)+1;
    static const int kPlaySize = kBoardSize*kBoardSize;
    static const int kMaxChains = (kPlaySize*3)/4;

    static uint8_t getSize() { return kBoardSize; }
    static int offset(Point p) { return (p.x()+1)+(p.y()+1)*(kBoardSize+1); }

    static typename Point::pod NatMap_for_IntSet(Point p) { return p.x()+p.y()*kBoardSize; }
    static Point COORD(int x, int y) { return Point::fromCoord(x,y); }

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

    NatMap<Point, BoardState> states;
    ChainIndex chain_indexes[kBoardArraySize];
    int chain_count;
    Point koPoint;
    PointSet emptyPoints;

    Board() : koPoint(COORD(-1,-1)) {
        reset();
    }

    void reset() {
        chain_count = 0;
        koPoint = COORD(-1,-1);
        states.setAll(BoardState::EMPTY());
        memset(chain_indexes, 0, sizeof(chain_indexes));
        for(int y=-1; y<=(int)kBoardSize; y++) {
            bs(COORD(-1, y)) = BoardState::WALL();
            bs(COORD(kBoardSize, y)) = BoardState::WALL();
        }
        for(int x=-1; x<=(int)kBoardSize; x++) {
            bs(COORD(x, -1)) = BoardState::WALL();
            bs(COORD(x, kBoardSize)) = BoardState::WALL();
        }
        emptyPoints.reset();
        for(int x=0; x<kBoardSize; x++) {
            for(int y=0; y<kBoardSize; y++) {
                emptyPoints.add(COORD(x,y));
            }
        }
    }

    void assertGoodState() {
        //walls are intact
        for(int y=-1; y<=(int)kBoardSize; y++) {
            ASSERT(bs(COORD(-1, y)) == BoardState::WALL());
            ASSERT(bs(COORD(kBoardSize, y)) == BoardState::WALL());
        }
        for(int x=-1; x<=(int)kBoardSize; x++) {
            ASSERT(bs(COORD(x, -1)) == BoardState::WALL());
            ASSERT(bs(COORD(x, kBoardSize)) == BoardState::WALL());
        }
        //empty points are correct
        for(int y=0; y<kBoardSize; y++) {
            for(int x=0; x<kBoardSize; x++) {
                Point p = COORD(x,y);
                if(bs(p) == BoardState::EMPTY()) {
                    ASSERT(emptyPoints.contains(p));
                } else {
                    ASSERT(!emptyPoints.contains(p));
                }
            }
        }
    }

    void dump() {
        for(int y=-1; y<=(int)kBoardSize; y++) {
            for(int x=-1; x<=(int)kBoardSize; x++) {
                putc(bs(COORD(x,y)).stateChar(), stdout);
            }
            putc('\n', stdout);
        }
        fflush(stdout);
    }

    BoardState& bs(Point p) { return states[p]; }
    const BoardState& bs(Point p) const { return states[p]; }

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
                Point lp = COORD(x,y);
                if(bs(lp) != BoardState::EMPTY()) continue;
#define doit(D) if(chain_indexes[offset(lp.D())] == chain_indexes[offset(p)]) { result++; continue; }
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
        if(bs(p.D()) == BoardState::EMPTY()) { chainAt(chain).addLiberty(p.D()); } \
        else if(bs(p.D()) == bs(p)) { mergeChains(chain, p.D()); }
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
#define doit(D) if(bs(p.D()) == BoardState::EMPTY()) { c.addLiberty(p.D()); }
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
#define doit(D) if(bs(p.D()) == BoardState::BLACK() || bs(p.D()) == BoardState::WHITE()) { chainAddLiberty(p.D(), p); }
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
        koPoint = COORD(-1, -1);

        bs(p) = c;
        if(false) {}
#define doit(D) else if(bs(p.D()) == c) { chainAddPoint(p.D(), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        else {
            makeNewChain(p);
        }

        BoardState ec = c.enemy();
#define doit(D) if(bs(p.D()) == ec) { chainRemoveLiberty(p.D(), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        emptyPoints.remove(p);
    }

    bool isSuicide(BoardState c, Point p) const {
#define doit(D) if(bs(p.D()) == BoardState::EMPTY()) { return false; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        BoardState ec = c.enemy();
#define doit(D) if(bs(p.D()) == ec && chainAt(p.D()).isInAtari()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

#define doit(D) if(bs(p.D()) == c && !chainAt(p.D()).isInAtari()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        return true;
    }

    bool isSimpleEye(BoardState c, Point p) const {
        if(bs(p) != BoardState::EMPTY()) return false;
#define doit(D) if(bs(p.D()) != c && bs(p.D()) != BoardState::WALL()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        BoardState ec = c.enemy();
        int enemy_diagonal_count = 0;
#define doit(D,F) if(bs(p.D().F()) == ec) { enemy_diagonal_count++; }
        doit(N,E)
        doit(N,W)
        doit(S,E)
        doit(S,W)
#undef doit
        if(enemy_diagonal_count == 0) return true;
        if(enemy_diagonal_count >= 2) return false;

        int wall_count = 0;
#define doit(D) if(bs(p.D()) == BoardState::WALL()) { wall_count++; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        if(wall_count>1) return false;
        return true;
    }

    bool lastMoveWasKo() {
        return koPoint != COORD(-1,-1);
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
                Point p = COORD(x,y);
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


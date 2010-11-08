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

    Point() : Nat(-1) {
        ASSERT(!isValid());
    }
};

typedef uint8_t ChainIndex;

template<uint kBoardSize>
struct Board {
    typedef Point<kBoardSize> Point;

    static const int kPlaySize = kBoardSize*kBoardSize;

    static uint8_t getSize() { return kBoardSize; }

    static typename Point::pod NatMap_for_IntSet(Point p) { return p.x()+p.y()*kBoardSize; }
    static Point COORD(int x, int y) { return Point::fromCoord(x,y); }

    typedef IntSet<Point, kPlaySize, Board> PointSet;

    struct ChainInfo {
        uint liberty_count;
        uint liberty_sum;
        uint liberty_sum_squares;
        uint _size;

        void reset() {
            liberty_count = 0;
            liberty_sum = 0;
            liberty_sum_squares = 0;
            _size = 0;
        }
        uint size() const { return _size; }
        bool isDead() const { return liberty_count == 0; }

        void addStone(Point p) { _size++; }
        void addLiberty(Point p) {
            liberty_count++;
            liberty_sum += p.toUint();
            liberty_sum_squares += p.toUint()*p.toUint();
        }
        void removeLiberty(Point p) {
            liberty_count--;
            liberty_sum -= p.toUint();
            liberty_sum_squares -= p.toUint()*p.toUint();
        }
        void merge(ChainInfo& o) {
            _size += o._size;
            liberty_count += o.liberty_count;
            liberty_sum += o.liberty_sum;
            liberty_sum_squares += o.liberty_sum_squares;
        }
        bool isInAtari() const {
            return (liberty_count * liberty_sum_squares) == (liberty_sum * liberty_sum);
        }
    };

#define FOREACH_CHAIN_STONE(chainPt, pt, block) \
    { \
        Point orig = chainPt; \
        Point pt = chainPt; \
        do { \
            block \
            pt = chain_next_point[pt]; \
        } while(pt != orig); \
    }

    NatMap<Point, BoardState> states;
    NatMap<Point, Point> chain_next_point; //circular list
    NatMap<Point, Point> chain_ids; //one point is the 'master' of each chain, it is where the chain data gets stored
    NatMap<Point, ChainInfo> chain_infos;

    Point koPoint;
    PointSet emptyPoints;

    Board() {
        reset();
    }

    void reset() {
        koPoint = Point::invalid();
        chain_next_point.setAll(Point::invalid());
        chain_ids.setAll(Point::invalid());
        ForEachNat(Point, p) {
            chain_infos[p].reset();
        }

        states.setAll(BoardState::EMPTY());
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
        assertGoodState();
    }

    void assertGoodState() {
        if(!kCheckAsserts) return;
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

    void assertChainLibertiesAreCorrect(Point p) {
        Point chainPt = chain_ids[p];
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

    ChainInfo& chainInfoAt(Point p) { return chain_infos[chain_ids[p]]; }
    const ChainInfo& chainInfoAt(Point p) const { return chain_infos[chain_ids[p]]; }

    uint8_t countLiberties(Point p) const {
        if(bs(p) != BoardState::BLACK() && bs(p) != BoardState::WHITE()) return 0;
        uint8_t result = 0;
        for(uint y=0; y<kBoardSize; y++) {
            for(uint x=0; x<kBoardSize; x++) {
                Point lp = COORD(x,y);
                if(bs(lp) != BoardState::EMPTY()) continue;
#define doit(D) if(chain_ids[lp.D()] == chain_ids[p]) { result++; continue; }
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
        assertGoodState();

        if(chain_ids[dest] == chain_ids[inc]) return;
        chainInfoAt(dest).merge(chainInfoAt(inc));
        //make the incoming stones think they're part of the dest chain
        FOREACH_CHAIN_STONE(inc, p, {
            chain_ids[p] = chain_ids[dest];
        });
        //stitch together the circular lists
        Point i0 = inc;
        Point i1 = chain_next_point[i0];
        Point d0 = dest;
        Point d1 = chain_next_point[d0];
        chain_next_point[d0] = i1;
        chain_next_point[i0] = d1;

        assertGoodState();
    }

    void chainAddPoint(Point chain, Point p) {
        assertGoodState();

        chain_next_point[p] = chain_next_point[chain];
        chain_next_point[chain] = p;
        chain_ids[p] = chain_ids[chain];

        chainInfoAt(chain).addStone(p);

#define doit(D) \
        if(bs(p.D()) == BoardState::EMPTY()) { chainInfoAt(chain).addLiberty(p.D()); } \
        else if(bs(p.D()) == bs(p)) { mergeChains(chain, p.D()); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        chainInfoAt(chain).removeLiberty(p);

        assertGoodState();
    }

    void makeNewChain(Point p) {
        chain_next_point[p] = p;
        chain_ids[p] = p;

        ChainInfo& c = chainInfoAt(p);
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
        chainInfoAt(chain).addLiberty(p);
    }

    void chainRemoveLiberty(Point chainPt, Point p) {
        assertGoodState();

        ChainInfo& c = chainInfoAt(chainPt);
        c.removeLiberty(p);
        if(c.isDead()) {
            //kill
            FOREACH_CHAIN_STONE(chainPt, p, {
                bs(p) = BoardState::EMPTY();
                emptyPoints.add(p);
            });
#define doit(D) if(bs(p.D()) == BoardState::BLACK() || bs(p.D()) == BoardState::WHITE()) { chainAddLiberty(p.D(), p); }
            FOREACH_CHAIN_STONE(chainPt, p, {
                doit(N)
                doit(S)
                doit(E)
                doit(W)
            });
#undef doit
            FOREACH_CHAIN_STONE(chainPt, p, {
                chain_ids[p] = Point::invalid();
            });
            if(c.size() == 1) {
                koPoint = chainPt;
            }
        }

        assertGoodState();
    }

    void makeMoveAssumeLegal(BoardState c, Point p) {
        assertGoodState();

        koPoint = Point::invalid();

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

        assertGoodState();
    }

    bool isSuicide(BoardState c, Point p) const {
#define doit(D) if(bs(p.D()) == BoardState::EMPTY()) { return false; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        BoardState ec = c.enemy();
#define doit(D) if(bs(p.D()) == ec && chainInfoAt(p.D()).isInAtari()) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

#define doit(D) if(bs(p.D()) == c && !chainInfoAt(p.D()).isInAtari()) return false;
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
        return koPoint.isValid();
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


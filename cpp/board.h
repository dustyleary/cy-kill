#pragma once

struct PlayoutResults {
    uint black_wins;
    uint white_wins;
    uint total_moves;
    uint millis_taken;
};

struct Board {
    Point koPoint;
    PointSet emptyPoints;
    BoardState lastPlayerColor;
    Point lastMove;
    uint size;

    NatMap<Point, BoardState> states;
    NatMap<Point, Point> chain_next_point; //circular list
    NatMap<Point, Point> chain_ids; //one point is the 'master' of each chain, it is where the chain data gets stored
    NatMap<Point, ChainInfo> chain_infos;
    NatMap<Point, Pat3> pat3s;

#define FOREACH_BOARD_POINT(p, block) \
    for(uint y=0; y<getSize(); y++) { \
        for(uint x=0; x<getSize(); x++) { \
            Point p = COORD(x,y); \
            block \
        } \
    }

    Board(uint s) : size(s) {
        reset();
    }

    Board(const Board& other) {
        memcpy(this, &other, sizeof(Board));
    }

    Board& operator=(const Board& other) {
        memcpy(this, &other, sizeof(Board));
        return *this;
    }

    void reset() {
        koPoint = Point::invalid();
        chain_next_point.setAll(Point::invalid());
        chain_ids.setAll(Point::invalid());
        FOREACH_NAT(Point, p, {
            chain_infos[p].reset();
        });

        states.setAll(BoardState::EMPTY());
        for(int y=-1; y<=(int)getSize(); y++) {
            bs(COORD(-1, y)) = BoardState::WALL();
            bs(COORD(getSize(), y)) = BoardState::WALL();
        }
        for(int x=-1; x<=(int)getSize(); x++) {
            bs(COORD(x, -1)) = BoardState::WALL();
            bs(COORD(x, getSize())) = BoardState::WALL();
        }
        emptyPoints.reset();
        for(int x=0; x<getSize(); x++) {
            for(int y=0; y<getSize(); y++) {
                emptyPoints.add(COORD(x,y));
            }
        }
        assertGoodState();
    }

    uint getSize() const {
        return size;
    }

    void assertGoodState() {
        if(!kCheckAsserts) return;
        //walls are intact
        for(int y=-1; y<=(int)getSize(); y++) {
            ASSERT(bs(COORD(-1, y)) == BoardState::WALL());
            ASSERT(bs(COORD(getSize(), y)) == BoardState::WALL());
        }
        for(int x=-1; x<=(int)getSize(); x++) {
            ASSERT(bs(COORD(x, -1)) == BoardState::WALL());
            ASSERT(bs(COORD(x, getSize())) == BoardState::WALL());
        }
        for(int y=0; y<getSize(); y++) {
            for(int x=0; x<getSize(); x++) {
                Point p = COORD(x,y);

                //empty points are correct
                if(bs(p) == BoardState::EMPTY()) {
                    ASSERT(emptyPoints.contains(p));
                } else {
                    ASSERT(!emptyPoints.contains(p));
                }

                if(bs(p).isPlayer()) {
                    assertChainGoodState(p);
                }
            }
        }
    }

    void assertChainGoodState(Point p) {
        Point chainPt = chain_ids[p];
        ChainInfo ci2;
        uint stone_count = 0;
        for(int y=0; y<getSize(); y++) {
            for(int x=0; x<getSize(); x++) {
                Point lp = COORD(x,y);
                if(chain_ids[lp] == chain_ids[p]) {
                    ++stone_count;
                }
                if(bs(lp) == BoardState::EMPTY()) {
                    FOREACH_POINT_DIR(lp, d, if(chain_ids[d] == chain_ids[chainPt]) { ci2.addLiberty(lp); })
                }
            }
        }
        ChainInfo& ci = chainInfoAt(p);
        ASSERT(ci.liberty_count == ci2.liberty_count);
        ASSERT(ci.liberty_sum == ci2.liberty_sum);
        ASSERT(ci.liberty_sum_squares == ci2.liberty_sum_squares);
        ASSERT(stone_count == ci._size);
    }

    void dump() {
        for(int y=-1; y<=(int)getSize(); y++) {
            for(int x=-1; x<=(int)getSize(); x++) {
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
        for(uint y=0; y<getSize(); y++) {
            for(uint x=0; x<getSize(); x++) {
                Point lp = COORD(x,y);
                if(bs(lp) != BoardState::EMPTY()) continue;
                FOREACH_POINT_DIR(lp, d, if(chain_ids[d] == chain_ids[p]) { result++; continue; })
            }
        }
        return result;
    }

    template<uint N>
    Pattern<N> calculatePatternAt(Point p) const {
        Pattern<N> result;
        for(int y=0; y<N; y++) {
            for(int x=0; x<N; x++) {
                BoardState c = BoardState::WALL();
                Point pp = COORD(p.x()-N/2+x, p.y()-N/2+y);
                if(isOnBoard(pp)) {
                    c = bs(pp);
                }
                result.setColorAt(COORD(x,y), c);
            }
        }
        result.setAtaris(
            isInAtari(p.N()),
            isInAtari(p.S()),
            isInAtari(p.E()),
            isInAtari(p.W())
        );
        return result;
    }

    void mergeChains(Point dest, Point inc) {
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
    }

    void makeNewChain(Point p) {
        chain_next_point[p] = p;
        chain_ids[p] = p;

        ChainInfo& c = chainInfoAt(p);
        c.reset();
        c.addStone(p);

        FOREACH_POINT_DIR(p, d, if(bs(d) == BoardState::EMPTY()) { c.addLiberty(d); })
    }

    void chainAddLiberty(Point chain, Point p) {
        chainInfoAt(chain).addLiberty(p);
    }

    void chainRemoveLiberty(Point chainPt, Point p) {
        ChainInfo& c = chainInfoAt(chainPt);
        c.removeLiberty(p);
        if(c.isDead()) {
            //kill
            FOREACH_CHAIN_STONE(chainPt, p, {
                bs(p) = BoardState::EMPTY();
                emptyPoints.add(p);
            });

            FOREACH_CHAIN_STONE(chainPt, p, {
                FOREACH_POINT_DIR(p, d, {
                    if(bs(d).isPlayer()) {
                        chainAddLiberty(d, p);
                    }
                })
            });

            FOREACH_CHAIN_STONE(chainPt, p, {
                chain_ids[p] = Point::invalid();
            });
            if(c.size() == 1) {
                koPoint = chainPt;
            }
        }
    }

    bool isInAtari(Point p) const {
        if(!bs(p).isPlayer()) return false;
        return chainInfoAt(p).isInAtari();
    }

    Point getAtariVertex(Point p) const {
        if(!bs(p).isPlayer()) return Point::invalid();
        const ChainInfo& c = chainInfoAt(p);
        if(!c.isInAtari()) return Point::invalid();
        return c.atariVertex();
    }

    void playMoveAssumeLegal(BoardState c, Point p) {
        if(p == Point::pass()) {
            lastPlayerColor = c;
            lastMove = p;
            return;
        }

        koPoint = Point::invalid();

        bs(p) = c;
        makeNewChain(p);
        emptyPoints.remove(p);

        //remove this point from my other chains' liberties
        FOREACH_POINT_DIR(p, d, if(bs(d) == c) { chainInfoAt(d).removeLiberty(p); })

        //remove this point from enemy chains' liberties (and perhaps kill)
        FOREACH_POINT_DIR(p, d, if(bs(d) == c.enemy()) { chainRemoveLiberty(d, p); })

        //merge with other chains
        FOREACH_POINT_DIR(p, d, if(bs(d) == c) { mergeChains(d, p); })

        lastPlayerColor = c;
        lastMove = p;

        assertGoodState();
    }

    bool isSuicide(BoardState c, Point p) const {
        FOREACH_POINT_DIR(p, d, if(bs(d) == BoardState::EMPTY()) { return false; })

        BoardState ec = c.enemy();
        FOREACH_POINT_DIR(p, d, if(bs(d) == ec && chainInfoAt(d).isInAtari()) return false;)

        FOREACH_POINT_DIR(p, d, if(bs(d) == c && !chainInfoAt(d).isInAtari()) return false;)

        return true;
    }

    bool isSimpleEye(BoardState c, Point p) const {
        if(bs(p) != BoardState::EMPTY()) return false;
        FOREACH_POINT_DIR(p, d, if(bs(d) != c && bs(d) != BoardState::WALL()) return false;)

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
        FOREACH_POINT_DIR(p, d, if(bs(d) == BoardState::WALL()) { wall_count++; })
        if(wall_count>1) return false;
        return true;
    }

    bool lastMoveWasKo() const {
        return koPoint.isValid();
    }

    bool isOnBoard(Point p) const {
        return p.x() < getSize() && p.y() < getSize();
    }

    bool isValidMove(BoardState c, Point p) const {
        if(p == Point::pass()) return true;
        if(!isOnBoard(p)) return false;
        if(bs(p) != BoardState::EMPTY()) return false;
        if(isSuicide(c, p)) return false;
        if(p == koPoint) return false;
        return true;
    }

    bool isValidMcgMove(BoardState c, Point p) const {
        return isValidMove(c, p) && !isSimpleEye(c,p);
    }

    void mcgMoves(BoardState c, PointSet& ps) const {
        ps.reset();
        for(int y=0; y<getSize(); y++) {
            for(int x=0; x<getSize(); x++) {
                Point p = COORD(x,y);
                if(isValidMcgMove(c, p)) {
                    ps.add(p);
                }
            }
        }
    }

    Point getRandomMove(BoardState c) {
        if(!emptyPoints.size()) {
            return Point::pass();
        }

        uint32_t mi = ::gen_rand32() % emptyPoints.size();
        uint32_t si = mi;
        while(true) {
            Point p = emptyPoints[mi];
            if(isValidMcgMove(c, p)) {
                return p;
            }
            mi = (mi + 1) % emptyPoints.size();
            if(mi == si) {
                return Point::pass();
            }
        }
    }

    Point playRandomMove(BoardState c) {
        Point p = getRandomMove(c);
        playMoveAssumeLegal(c, p);
        return p;
    }

    int trompTaylorScore() const {
        int blackStones = 0;
        int whiteStones = 0;
        NatMap<Point, uint> reaches(0);

        FOREACH_BOARD_POINT(p, {
            blackStones += bs(p) == BoardState::BLACK();
            whiteStones += bs(p) == BoardState::WHITE();
            if(bs(p) == BoardState::EMPTY()) {
                FOREACH_POINT_DIR(p, d, {
                    if(bs(d) == BoardState::BLACK()) { reaches[p] |= 1; }
                    else if(bs(d) == BoardState::WHITE()) { reaches[p] |= 2; }
                })
            }
        });

        bool coloredSome;
        do {
            coloredSome = false;
            FOREACH_BOARD_POINT(p, {
                if(bs(p) == BoardState::EMPTY()) {
                    FOREACH_POINT_DIR(p, d, {
                        if((0==(reaches[p]&1)) && (reaches[d]&1)) { reaches[p] |= 1; coloredSome = true; }
                        if((0==(reaches[p]&2)) && (reaches[d]&2)) { reaches[p] |= 2; coloredSome = true; }
                    })
                }
            });
        } while(coloredSome);

        int whiteTerritory = 0;
        int blackTerritory = 0;
        FOREACH_BOARD_POINT(p, {
            blackTerritory += reaches[p] == 1;
            whiteTerritory += reaches[p] == 2;
        });
        return (whiteStones + whiteTerritory) - (blackStones + blackTerritory);
    }

    void doPlayouts_random(uint num_playouts, float komi, BoardState player_color, PlayoutResults& r) {
        r.black_wins = 0;
        r.white_wins = 0;

        uint32_t st = millisTime();
        Board playout_board(*this);

        int neg_komi = (int)(-floor(komi));

        for(uint i=0; i<num_playouts; i++) {
            memcpy(&playout_board, this, sizeof(Board));

            int passes = 0;
            int kos = 0;
            while(true) {
                Point p = playout_board.playRandomMove(player_color);
                r.total_moves++;
                if(p == Point::pass()) {
                    passes++;
                    if(passes>=2) {
                        break;
                    }
                } else {
                    passes = 0;
                }
                if(playout_board.lastMoveWasKo()) {
                    kos++;
                    if(kos > 4) {
                        break;
                    }
                } else {
                    kos = 0;
                }
                player_color = player_color.enemy();
            }
            if(playout_board.trompTaylorScore() < neg_komi) {
                r.black_wins++;
            } else {
                r.white_wins++;
            }
        }
        uint32_t et = millisTime();
        r.millis_taken = et-st;
    }
};


#pragma once

struct PlayoutResults {
    uint black_wins;
    uint white_wins;
    uint total_moves;
    uint millis_taken;
    PlayoutResults()
        : black_wins(0)
        , white_wins(0)
        , total_moves(0)
        , millis_taken(0)
    {
    }
};

struct Board : public TwoPlayerGridGame {
    Point koPoint;

    PointSet emptyPoints;
    float komi;

    NatMap<Point, Pattern<3> > pat3cache;
    PointSet pat3dirty;

    NatMap<Point, Point> chain_next_point; //circular list
    NatMap<Point, Point> chain_ids; //one point is the 'master' of each chain, it is where the chain data gets stored
    NatMap<Point, ChainInfo> chain_infos;

    Board(uint s, float komi=6.5f) : TwoPlayerGridGame(s), komi(komi) {
        reset();
    }

    void reset() {
        TwoPlayerGridGame::reset();

        koPoint = Point::invalid();
        chain_next_point.setAll(Point::invalid());
        chain_ids.setAll(Point::invalid());
        FOREACH_NAT(Point, p, {
            chain_infos[p].reset();
        });
        lastMove = Move();

        emptyPoints.reset();
        FOREACH_BOARD_POINT(p, {
            emptyPoints.add(p);
        });
        FOREACH_NAT(Point, p, {
            pat3cache[p] = _calculatePatternAt<3>(p);
        });
        assertGoodState();
    }

    void assertGoodState() {
        if(!kCheckAsserts) return;
        //walls are intact
        for(int y=-1; y<=(int)getSize(); y++) {
            ASSERT(bs(COORD(-1, y)) == PointColor::WALL());
            ASSERT(bs(COORD(getSize(), y)) == PointColor::WALL());
        }
        for(int x=-1; x<=(int)getSize(); x++) {
            ASSERT(bs(COORD(x, -1)) == PointColor::WALL());
            ASSERT(bs(COORD(x, getSize())) == PointColor::WALL());
        }
        for(int y=0; y<getSize(); y++) {
            for(int x=0; x<getSize(); x++) {
                Point p = COORD(x,y);

                //empty points are correct
                if(bs(p) == PointColor::EMPTY()) {
                    ASSERT(emptyPoints.contains(p));
                } else {
                    ASSERT(!emptyPoints.contains(p));
                }

                if(bs(p).isPlayer()) {
                    assertChainGoodState(p);
                }
            }
        }
        FOREACH_NAT(Point, p, {
            if(bs(p) == PointColor::EMPTY()) {
                //LOG("assertPat3CacheGoodState: % 4d (% 2d,% 2d) %s", p.v, p.x(), p.y(), p.toGtpVertex(getSize()).c_str());
                Pattern<3> goodpat = _calculatePatternAt<3>(p);
                Pattern<3> testpat = pat3cache[p];
                //goodpat.resetAtaris();
                //testpat.resetAtaris();
                //goodpat.dump();
                //testpat.dump();
                ASSERT(goodpat == testpat);
            }
        });
    }

    void assertChainGoodState(Point p) {
        //LOG("assertChainGoodState(): v: % 4d (% 2d,% 2d) %s", p.v, p.x(), p.y(), p.toGtpVertex(getSize()).c_str());
        Point chainPt = chain_ids[p];
        ChainInfo ci2;
        for(int y=0; y<getSize(); y++) {
            for(int x=0; x<getSize(); x++) {
                Point lp = COORD(x,y);
                if(chain_ids[lp] == chain_ids[p]) {
                    ci2.addStone(p);
                }
                if(bs(lp) == PointColor::EMPTY()) {
                    FOREACH_POINT_DIR(lp, d, if(chain_ids[d] == chain_ids[chainPt]) { ci2.addLiberty(lp); })
                }
            }
        }
        ChainInfo& ci = chainInfoAt(p);
        ASSERT(ci.liberty_count == ci2.liberty_count);
        ASSERT(ci.liberty_sum == ci2.liberty_sum);
        ASSERT(ci.liberty_sum_squares == ci2.liberty_sum_squares);
        ASSERT(ci._size == ci2._size);
    }

    void playStone(Point p, PointColor c) {
        ASSERT(c.isPlayer());
        set_bs(p, c);
        emptyPoints.remove(p);
        updatePat3x3Colors(p);
    }
    void removeStone(Point p) {
        set_bs(p, PointColor::EMPTY());
        emptyPoints.add(p);
        updatePat3x3Colors(p);
        pat3cache[p].resetAtaris();
    }

    ChainInfo& chainInfoAt(Point p) { return chain_infos[chain_ids[p]]; }
    const ChainInfo& chainInfoAt(Point p) const { return chain_infos[chain_ids[p]]; }

    uint8_t countLiberties(Point p) const {
        if(bs(p) != PointColor::BLACK() && bs(p) != PointColor::WHITE()) return 0;
        uint8_t result = 0;
        for(uint y=0; y<getSize(); y++) {
            for(uint x=0; x<getSize(); x++) {
                Point lp = COORD(x,y);
                if(bs(lp) != PointColor::EMPTY()) continue;
                FOREACH_POINT_DIR(lp, d, if(chain_ids[d] == chain_ids[p]) { result++; continue; })
            }
        }
        return result;
    }

    Pat3 getPatternAt(Point p) const {
        return pat3cache[p];
    }

    template<uint N>
    Pattern<N> _calculatePatternAt(Point p) const {
        Pattern<N> result;
        for(int y=0; y<N; y++) {
            for(int x=0; x<N; x++) {
                if(x == Pattern<N>::mid() && y == Pattern<N>::mid()) {
                    continue;
                }
                PointColor c = PointColor::WALL();
                int px = p.x()-N/2+x;
                int py = p.y()-N/2+y;
                Point pp = COORD(px, py);
                if((px>=0) && (py>=0) && (px<getSize()) && (py<getSize())) {
                    c = bs(pp);
                }
                result.setColorAt(x,y, c);
            }
        }
        int px = p.x();
        int py = p.y();
        if((px>=0) && (py>=0) && (px<getSize()) && (py<getSize())) {
            result.setAtaris(
                isInAtari(p.N()),
                isInAtari(p.S()),
                isInAtari(p.E()),
                isInAtari(p.W())
            );
        }
        return result;
    }

    template<uint N>
    Pattern<N> canonicalPatternAt(PointColor c, Point _p) const {
        Pattern<N> p = _calculatePatternAt<N>(_p);
        if(c == PointColor::WHITE()) {
            p = p.invert_colors();
        }
        return p.canonical();
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

    uint64_t zobrist() const {
        uint64_t r = boardHash();

        //ko point
        if(koPoint != Point::invalid()) {
          r ^= Zobrist::black[koPoint];
          r ^= Zobrist::white[koPoint];
        }
        //whos turn it is
        Point turnPoint = COORD(5,-1);
        if(getWhosTurn() == PointColor::BLACK()) {
          r ^= Zobrist::black[turnPoint];
        } else {
          r ^= Zobrist::white[turnPoint];
        }
        return r;
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

    void checkLeaveAtari(ChainInfo& c, Point anyChainPt) {
        if(!c.isInAtari()) return;
        //LOG("checkLeaveAtari");
        Point atariVertex = c.atariVertex();

        pat3cache[atariVertex].clearAtaris(
            chain_ids[atariVertex.N()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.S()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.E()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.W()] == chain_ids[anyChainPt]
        );
        pat3dirty.add(atariVertex);
    }

    bool checkEnterAtari(ChainInfo& c, Point anyChainPt) {
        if(!c.isInAtari()) return false;
        //LOG("checkEnterAtari");
        Point atariVertex = c.atariVertex();

        pat3cache[atariVertex].setAtaris(
            chain_ids[atariVertex.N()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.S()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.E()] == chain_ids[anyChainPt],
            chain_ids[atariVertex.W()] == chain_ids[anyChainPt]
        );
        pat3dirty.add(atariVertex);
        return true;
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

    void updatePat3x3Colors(Point p) {
        PointColor c = bs(p);
        int px = p.x();
        int py = p.y();
        pat3cache[COORD(px-1,py-1)].setColorAt( 2, 2, c);
        pat3cache[COORD(px-0,py-1)].setColorAt( 1, 2, c);
        pat3cache[COORD(px+1,py-1)].setColorAt( 0, 2, c);

        pat3cache[COORD(px-1,py-0)].setColorAt( 2, 1, c);
        pat3cache[COORD(px+1,py-0)].setColorAt( 0, 1, c);

        pat3cache[COORD(px-1,py+1)].setColorAt( 2, 0, c);
        pat3cache[COORD(px-0,py+1)].setColorAt( 1, 0, c);
        pat3cache[COORD(px+1,py+1)].setColorAt( 0, 0, c);

        Point d;

        d = COORD(px-1,py-1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
        d = COORD(px-0,py-1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
        d = COORD(px+1,py-1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);

        d = COORD(px-1,py-0); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
        d = COORD(px-0,py-0);                                  pat3dirty.add(d);
        d = COORD(px+1,py-0); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);

        d = COORD(px-1,py+1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
        d = COORD(px-0,py+1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
        d = COORD(px+1,py+1); if(bs(d) == PointColor::EMPTY()) pat3dirty.add(d);
    }

    void killChain(Point chainPt) {
        Point p = chainPt;
        PointColor e = bs(chainPt).enemy();
        ChainInfo& c = chainInfoAt(chainPt);
        FOREACH_CHAIN_STONE(chainPt, p, {
            removeStone(p);
            FOREACH_POINT_DIR(p, d, {
                if(bs(d) == e) {
                    checkLeaveAtari(chainInfoAt(d), d);
                    chainInfoAt(d).addLiberty(p);
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

    void makeNewChain(Point p) {
        chain_next_point[p] = p;
        chain_ids[p] = p;

        ChainInfo& c = chainInfoAt(p);
        c.reset();
        c.addStone(p);

        FOREACH_POINT_DIR(p, d, {
            if(bs(d) == PointColor::EMPTY()) {
                //either add a liberty to my new chain
                c.addLiberty(d);
            } else if(bs(d).isPlayer()) {
                //or remove a liberty from neighboring chain
                chainInfoAt(d).removeLiberty(p);
            }
        })
    }

    void _playMoveAssumeLegal(Move m) {
        ASSERT(!isSuicide(m));
        //ASSERT(m.point != koPoint);
        //LOG("playMove: %s", p.toGtpVertex(getSize()).c_str());
        if(m.point != Point::pass()) {
            ASSERT(bs(m.point) == PointColor::EMPTY());

            koPoint = Point::invalid();

            //dump();

            playStone(m.point, m.playerColor);
            makeNewChain(m.point);

            FOREACH_POINT_DIR(m.point, d, {
                if(bs(d) == m.playerColor) {
                    //friendly neighbor, merge chains
                    mergeChains(d, m.point);
                } else if(bs(d) == m.playerColor.enemy()) {
                    //enemy neighbor, liberty has already been removed
                    ChainInfo& ec = chainInfoAt(d);
                    if(ec.isDead()) {
                        killChain(d);
                    } else if(ec.isInAtari()) {
                        //chain just went into atari, update pat3 cache
                        checkEnterAtari(ec, d);
                    }
                }
            });

            bool enteredAtari = checkEnterAtari(chainInfoAt(m.point), m.point);
            if(enteredAtari) {
              //IFF we captured an enemy chain (above), then we may have set a koPoint,
              //and we may have entered atari...
              //if we have entered atari and our chain is bigger than 1,
              //then we can be snap-backed (and the ko point
              //is invalid)
              if(chainInfoAt(m.point).size() > 1) {
                koPoint = Point::invalid();
              }
            }
        }
    }

    bool isSuicide(PointColor c, Point p) const {
      return isSuicide(Move(c, p));
    }

    bool isSuicide(Move m) const {
        if(m.point == Point::pass()) return false;

        FOREACH_POINT_DIR(m.point, d, if(bs(d) == PointColor::EMPTY()) { return false; })

        PointColor ec = m.playerColor.enemy();
        FOREACH_POINT_DIR(m.point, d, if(bs(d) == ec && chainInfoAt(d).isInAtari()) return false;)

        FOREACH_POINT_DIR(m.point, d, if(bs(d) == m.playerColor && !chainInfoAt(d).isInAtari()) return false;)

        return true;
    }

    bool isSimpleEye(Move m) const {
        if(bs(m.point) != PointColor::EMPTY()) return false;
        FOREACH_POINT_DIR(m.point, d, if(bs(d) != m.playerColor && bs(d) != PointColor::WALL()) return false;)

        NatMap<PointColor, uint> diagonal_counts(0);
#define doit(D,F) diagonal_counts[bs(m.point.D().F())]++;
        doit(N,E)
        doit(N,W)
        doit(S,E)
        doit(S,W)
#undef doit

        PointColor ec = m.playerColor.enemy();
        return (diagonal_counts[ec] + (diagonal_counts[PointColor::WALL()]>0)) < 2;
    }

    bool lastMoveWasKo() const {
        return koPoint.isValid();
    }

    bool isValidMove(Move m) const {
        if(m.point == Point::pass()) return true;
        if(!isOnBoard(m.point)) return false;
        if(bs(m.point) != PointColor::EMPTY()) return false;
        if(isSuicide(m)) return false;
        if(m.point == koPoint) return false;
        return true;
    }

    void getValidMoves(PointColor c, std::vector<Move>& out, uint moduloNumerator=0, uint moduloDenominator=1) const {
        out.clear();
        out.reserve(emptyPoints.size()+1);
        uint j=0;
        for(int i=0; i<emptyPoints.size(); i++) {
            Point p = emptyPoints[i];
            if(isSuicide(c, p)) continue;
            if(p == koPoint) continue;
            if(j++ % moduloDenominator != moduloNumerator) continue;
            out.push_back(Move(c, p));
        }
        out.push_back(Move(c, Point::pass()));
    }

    template<uint N>
    std::map<Point, Pattern<N> > getCanonicalPatternsForValidMoves(PointColor c) {
      std::vector<Move> moves;
      getValidMoves(c, moves);

      std::map<Point, Pattern<N> > result;
      for(uint i=0; i<moves.size(); i++) {
        Point p = moves[i].point;
        result[p] = canonicalPatternAt<N>(c, p);
      }

      return result;
    }

    float trompTaylorScore() const {
        int blackStones = 0;
        int whiteStones = 0;
        NatMap<Point, uint> reaches(0);

        FOREACH_BOARD_POINT(p, {
            blackStones += bs(p) == PointColor::BLACK();
            whiteStones += bs(p) == PointColor::WHITE();
            if(bs(p) == PointColor::EMPTY()) {
                FOREACH_POINT_DIR(p, d, {
                    if(bs(d) == PointColor::BLACK()) { reaches[p] |= 1; }
                    else if(bs(d) == PointColor::WHITE()) { reaches[p] |= 2; }
                })
            }
        });

        bool coloredSome;
        do {
            coloredSome = false;
            FOREACH_BOARD_POINT(p, {
                if(bs(p) == PointColor::EMPTY()) {
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
        return komi + (whiteStones + whiteTerritory) - (blackStones + blackTerritory);
    }
};
typedef Board::Move Move;


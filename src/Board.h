#pragma once

struct Board : public TwoPlayerGridGame {
    bool use_gamma_random_player;

    Point koPoint;

    PointSet emptyPoints;
    float komi;
    int consecutivePasses;
    int consecutiveKoMoves;

    NatMap<Point, double> _pat3cacheGamma[2];
    PointSet _pat3dirty;
    double gammaSum[2];

    NatMap<Point, Point> chain_next_point; //circular list
    NatMap<Point, Point> chain_ids; //one point is the 'master' of each chain, it is where the chain data gets stored
    NatMap<Point, ChainInfo> chain_infos;

    Board(uint s=19, float komi=6.5f) : TwoPlayerGridGame(s), komi(komi) {
        use_gamma_random_player = true;
        reset();
    }

    void reset() {
        TwoPlayerGridGame::reset();
        consecutivePasses = 0;
        consecutiveKoMoves = 0;

        koPoint = Point::invalid();
        chain_next_point.setAll(Point::invalid());
        chain_ids.setAll(Point::invalid());
        FOREACH_NAT(Point, p, {
            chain_infos[p].reset();
        });

        emptyPoints.reset();
        FOREACH_BOARD_POINT(p, {
            emptyPoints.add(p);
        });
        if(use_gamma_random_player) {
            recalculate_all_pat3_gammas();
        }
        assertGoodState();
    }

    void setGammaPlayer(bool use_gamma) {
        use_gamma_random_player = use_gamma;
        recalculate_all_pat3_gammas();
    }

    void recalculate_all_pat3_gammas() {
        gammaSum[0] = gammaSum[1] = 0;
        _pat3cacheGamma[0].setAll(0);
        _pat3cacheGamma[1].setAll(0);
        FOREACH_BOARD_POINT(p, {
            Pat3 pattern = _calculatePatternAt<3>(p);
            _pat3cacheGamma[0][p] = getPat3Gamma(pattern);
            _pat3cacheGamma[1][p] = getPat3Gamma(pattern.invert_colors());
            gammaSum[0] += _pat3cacheGamma[0][p];
            gammaSum[1] += _pat3cacheGamma[1][p];
        });
        _pat3dirty.reset();
    }

    inline void dirtyPat3Cache(Point p) {
        if(!use_gamma_random_player) return;
        if(!isOnBoard(p)) return;
        _pat3dirty.add(p);
        //LOG("dirty: %s", p.toGtpVertex().c_str());
    }

    void recalcDirtyPat3s() {
        if(!use_gamma_random_player) return;
        //LOG("recalcDirtyPat3s");
        for(uint i=0; i<_pat3dirty.size(); i++) {
            Point p = _pat3dirty[i];
            //LOG("recalc: %s", p.toGtpVertex().c_str());

            gammaSum[0] -= _pat3cacheGamma[0][p];
            gammaSum[1] -= _pat3cacheGamma[1][p];

            if(bs(p) != PointColor::EMPTY()) {
                _pat3cacheGamma[0][p] = 0;
                _pat3cacheGamma[1][p] = 0;
                continue;
            }

            Pat3 pattern = _calculatePatternAt<3>(p);
            _pat3cacheGamma[0][p] = getPat3Gamma(pattern);
            _pat3cacheGamma[1][p] = getPat3Gamma(pattern.invert_colors());
            gammaSum[0] += _pat3cacheGamma[0][p];
            gammaSum[1] += _pat3cacheGamma[1][p];
        }
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
        if(use_gamma_random_player) {
            //recalcDirtyPat3s();
            double want_sum[2] = {0,0};
            FOREACH_NAT(Point, p, {
                if(bs(p) == PointColor::EMPTY()) {
                    //LOG("assertPat3CacheGoodState: % 4d (% 2d,% 2d) %s", p.v, p.x(), p.y(), p.toGtpVertex(getSize()).c_str());
                    Pattern<3> pat = _calculatePatternAt<3>(p).canonical();
                    ASSERT(getPat3Gamma(pat) == _pat3cacheGamma[0][p]);
                    ASSERT(getPat3Gamma(pat.invert_colors()) == _pat3cacheGamma[1][p]);
                    want_sum[0] += getPat3Gamma(pat);
                    want_sum[1] += getPat3Gamma(pat.invert_colors());
                    //Pattern<3> testpat = _pat3cache[p];
                    //goodpat.resetAtaris();
                    //testpat.resetAtaris();
                    //goodpat.dump();
                    //testpat.dump();
                    //ASSERT(goodpat == testpat);
                } else {
                    ASSERT(0 == _pat3cacheGamma[0][p]);
                    ASSERT(0 == _pat3cacheGamma[1][p]);
                }
            });
            double delta[2] = {want_sum[0]-gammaSum[0], want_sum[1]-gammaSum[1]};
            //LOG("delta = { %f, %f }", delta[0], delta[1]);
            ASSERT(abs(delta[0]) < 0.000001);
            ASSERT(abs(delta[1]) < 0.000001);
        }
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

        dirtyPat3Cache(COORD(p.x()-1, p.y()-1));
        dirtyPat3Cache(COORD(p.x()  , p.y()-1));
        dirtyPat3Cache(COORD(p.x()+1, p.y()-1));
        dirtyPat3Cache(COORD(p.x()-1, p.y()  ));
        //dirtyPat3Cache(COORD(p.x()  , p.y()  ));
        dirtyPat3Cache(p);
        dirtyPat3Cache(COORD(p.x()+1, p.y()  ));
        dirtyPat3Cache(COORD(p.x()-1, p.y()+1));
        dirtyPat3Cache(COORD(p.x()  , p.y()+1));
        dirtyPat3Cache(COORD(p.x()+1, p.y()+1));
    }
    void removeStone(Point p) {
        set_bs(p, PointColor::EMPTY());
        emptyPoints.add(p);

        dirtyPat3Cache(COORD(p.x()-1, p.y()-1));
        dirtyPat3Cache(COORD(p.x()  , p.y()-1));
        dirtyPat3Cache(COORD(p.x()+1, p.y()-1));
        dirtyPat3Cache(COORD(p.x()-1, p.y()  ));
        //dirtyPat3Cache(COORD(p.x()  , p.y()  ));
        dirtyPat3Cache(p);
        dirtyPat3Cache(COORD(p.x()+1, p.y()  ));
        dirtyPat3Cache(COORD(p.x()-1, p.y()+1));
        dirtyPat3Cache(COORD(p.x()  , p.y()+1));
        dirtyPat3Cache(COORD(p.x()+1, p.y()+1));
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

    template<uint N>
    Pattern<N> _calculatePatternAt(Point p) const {
        Pattern<N> result;
        for(int y=0; y<N; y++) {
            for(int x=0; x<N; x++) {
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

    template<uint N> inline Pattern<N> canonicalPatternAt(PointColor c, Point _p) const;

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

        dirtyPat3Cache(atariVertex);
    }

    bool checkEnterAtari(ChainInfo& c, Point anyChainPt) {
        if(!c.isInAtari()) return false;
        //LOG("checkEnterAtari");
        Point atariVertex = c.atariVertex();

        dirtyPat3Cache(atariVertex);
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
        //LOG("playMove: %s", m.point.toGtpVertex().c_str());
        if(m.point != Point::pass()) {
            ASSERT(bs(m.point) == PointColor::EMPTY());

            koPoint = Point::invalid();

            //dump();

            playStone(m.point, m.color);
            makeNewChain(m.point);

            FOREACH_POINT_DIR(m.point, d, {
                if(bs(d) == m.color) {
                    //friendly neighbor, merge chains
                    mergeChains(d, m.point);
                } else if(bs(d) == m.color.enemy()) {
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
            consecutivePasses = 0;
            if(koPoint == Point::invalid()) {
                consecutiveKoMoves = 0;
            } else {
                consecutiveKoMoves++;
            }
        } else {
            consecutivePasses++;
            koPoint = Point::invalid();
        }
        recalcDirtyPat3s();
    }

    bool isGameFinished() const {
        return consecutivePasses >= 2 || consecutiveKoMoves > 4;
    }

    PointColor winner() const {
        if(!isGameFinished()) return PointColor::EMPTY();
        return trompTaylorScore() > 0 ? PointColor::WHITE() : PointColor::BLACK();
    }

    bool isSuicide(Move m) const {
      return isSuicide(m.color, m.point);
    }

    bool isSuicide(PointColor color, Point point) const {
        if(point == Point::pass()) return false;

        FOREACH_POINT_DIR(point, d, if(bs(d) == PointColor::EMPTY()) { return false; })

        PointColor ec = color.enemy();
        FOREACH_POINT_DIR(point, d, if(bs(d) == ec && chainInfoAt(d).isInAtari()) return false;)

        FOREACH_POINT_DIR(point, d, if(bs(d) == color && !chainInfoAt(d).isInAtari()) return false;)

        return true;
    }

    bool isSimpleEye(Move m) const {
        return isSimpleEye(m.color, m.point);
    }

    bool isSimpleEye(PointColor color, Point point) const {
        if(bs(point) != PointColor::EMPTY()) return false;
        FOREACH_POINT_DIR(point, d, if(bs(d) != color && bs(d) != PointColor::WALL()) return false;)

        NatMap<PointColor, uint> diagonal_counts(0);
#define doit(D,F) diagonal_counts[bs(point.D().F())]++;
        doit(N,E)
        doit(N,W)
        doit(S,E)
        doit(S,W)
#undef doit

        PointColor ec = color.enemy();
        return (diagonal_counts[ec] + (diagonal_counts[PointColor::WALL()]>0)) < 2;
    }

    bool lastMoveWasKo() const {
        return koPoint.isValid();
    }

    bool isValidMove(Move m) const { return isValidMove(m.color, m.point); }

    bool isValidMove(PointColor color, Point p) const {
        if(p == Point::pass()) return true;
        if(!isOnBoard(p)) return false;
        if(bs(p) != PointColor::EMPTY()) return false;
        if(isSuicide(color, p)) return false;
        if(p == koPoint) return false;
        return true;
    }

    Point closestStarPoint(Point p) const {
      int x, y;
      if(p.x() <= 6) { x = 3;
      } else if(p.x() >= 12) { x = 15;
      } else { x = 9; }
      if(p.y() <= 6) { y = 3;
      } else if(p.y() >= 12) { y = 15;
      } else { y = 9; }
      return COORD(x,y);
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

    void trompTaylorOwners(PointSet& black, PointSet& white) const {
        black.reset();
        white.reset();

        NatMap<Point, uint> reaches(0);

        FOREACH_BOARD_POINT(p, {
            if(bs(p) == PointColor::BLACK()) black.add(p);
            else if(bs(p) == PointColor::WHITE()) white.add(p);

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

        FOREACH_BOARD_POINT(p, {
            if(reaches[p] == 1) black.add(p);
            else if(reaches[p] == 2) white.add(p);
        });
    }

    float trompTaylorScore() const {
        PointSet black, white;
        trompTaylorOwners(black, white);
        return komi + white.size() - black.size();
    }

    bool _isGoodRandomMove(PointColor c, Point p) const {
        return true
            && (!isSuicide(c, p))
            && (p != koPoint)
            && (!isSimpleEye(c, p))
            ;
    }

    Move getRandomMove(PointColor c) {
        if(!emptyPoints.size()) return Move(c, Point::pass());
        if(use_gamma_random_player) return getGammaMove(c);

        uint32_t mi = (uint32_t)::gen_rand64() % emptyPoints.size();
        uint32_t si = mi;
        while(true) {
            Point p = emptyPoints[mi];
            if(isValidMove(c, p)
                && !isSimpleEye(c, p)) {
                return Move(c, p);
            }
            mi = (mi + 1) % emptyPoints.size();
            if(mi == si) {
                return Move(c, Point::pass());
            }
        }
    }

    inline Move getGammaMove(PointColor c);

    inline double cachedGammaAt(PointColor c, Point _p) const {
        int idx = (c == PointColor::BLACK()) ? 0 : 1;
        return _pat3cacheGamma[idx][_p];
    }
};

template<uint N>
inline Pattern<N> Board::canonicalPatternAt(PointColor c, Point _p) const {
    Pattern<N> p = _calculatePatternAt<N>(_p);
    if(c == PointColor::WHITE()) { p = p.invert_colors(); }
    return p.canonical();
}

inline Board::Move Board::getGammaMove(PointColor c) {
    int idx = (c == PointColor::BLACK()) ? 0 : 1;
    //std::vector<double> weights;
    //weights.reserve(emptyPoints.size());
    //double weights_sum = 0;
    //for(uint i=0; i<emptyPoints.size(); i++) {
    ////    Pat3 p = canonicalPatternAt<3>(c, emptyPoints[i]);
    ////    double w = getPat3Gamma(p);
    //    weights.push_back(1);
    //NatMap<Point, double> _pat3cacheGamma[2];
    //    weights_sum += 1;
    //}
    int i = WeightedRandomChooser::choose(Point::kBound, &_pat3cacheGamma[idx].a[0], gammaSum[idx]);
    int si = i;
    while(true) {
        Point p = Point::fromUint(i);
        if(isValidMove(c, p)
            &&!isSimpleEye(c, p)
        ) {
            return Move(c, p);
        }
        i = (i+1) % Point::kBound;
        if(i == si) {
            return Move(c, Point::pass());
        }
    }
}


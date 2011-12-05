#pragma once

typedef double (*GammaFunc)(uint patternId);

GammaFunc loadGammasFromFile(const char* filename);
GammaFunc fakeGammas();

struct GammaPlayer : public RandomPlayerBase {
    struct Weights {
        NatMap<Point, double> weights;
        double weight_total;
    };

    Weights black, white;
    Point koPoint;

    GammaFunc gammaFunc;

    GammaPlayer(GammaFunc gammaFunc) : gammaFunc(gammaFunc) {
    }

    void resetStateForNewBoard(Board& b) {
        black.weight_total = white.weight_total = 0;
        black.weights.setAll(0);
        white.weights.setAll(0);
        koPoint = b.koPoint;

        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];

            Pat3 black_pat = b.getPatternAt(p);
            double black_patWeight = gammaFunc(black_pat.toUint());
            black.weights[p] = black_patWeight;
            black.weight_total += black_patWeight;

            Pat3 white_pat = black_pat.invert_colors();
            double white_patWeight = gammaFunc(white_pat.toUint());
            white.weights[p] = white_patWeight;
            white.weight_total += white_patWeight;
        }
        //LOG("reset: %d empty points, weight total: %f", b.emptyPoints.size(), weight_total);
        b.pat3dirty.reset();
        assertGoodState(b);
    }

    void assertGoodState(Board& b) {
        if(!kCheckAsserts) return;
        //assert all weights are valid in the board
        FOREACH_NAT(Point, p, {
            if(black.weights[p] != 0) {
                Move m(BoardState::BLACK(), p);
                if(!b.isValidMove(m) || b.isSimpleEye(m)) {
                    b.dump();
                    fprintf(stderr, "invalid move: %s has a non-zero weight\n", m.point.toGtpVertex(b.getSize()).c_str());
                    Pat3 pat = b.getPatternAt(p);
                    fprintf(stderr, "pat:\n");
                    pat.dump();
                    ASSERT(b.isValidMove(m));
                }
            }
            if(white.weights[p] != 0) {
                Move m(BoardState::WHITE(), p);
                if(!b.isValidMove(m) || b.isSimpleEye(m)) {
                    b.dump();
                    fprintf(stderr, "invalid move: %s has a non-zero weight\n", m.point.toGtpVertex(b.getSize()).c_str());
                    Pat3 pat = b.getPatternAt(p);
                    fprintf(stderr, "pat:\n");
                    pat.dump();
                    ASSERT(b.isValidMove(m));
                }
            }
        });
        //assert all valid moves in the board have a weight
        for(uint x=0; x<b.getSize(); x++) {
            for(uint y=0; y<b.getSize(); y++) {
                Move m(BoardState::BLACK(), x,y);
                if(b.isValidMove(m) && !b.isSimpleEye(m)) {
                    if(black.weights[m.point] == 0.0) {
                        b.dump();
                        fprintf(stderr, "expect weight non-zero: %s\n", m.point.toGtpVertex(b.getSize()).c_str());
                        Pat3 pat = b.getPatternAt(m.point);
                        fprintf(stderr, "pat:\n");
                        pat.dump();

                        ASSERT(black.weights[m.point] != 0.0);
                    }
                } else {
                    ASSERT(black.weights[m.point] == 0.0);
                }
                m.playerColor = BoardState::WHITE();
                if(b.isValidMove(m) && !b.isSimpleEye(m)) {
                    if(white.weights[m.point] == 0.0) {
                        b.dump();
                        fprintf(stderr, "expect weight non-zero: %s\n", m.point.toGtpVertex(b.getSize()).c_str());
                        Pat3 pat = b.getPatternAt(m.point);
                        fprintf(stderr, "pat:\n");
                        pat.dump();

                        ASSERT(white.weights[m.point] != 0.0);
                    }
                } else {
                    ASSERT(white.weights[m.point] == 0.0);
                }
            }
        }
    }

    void updatePointWeights(Board& b, Point p) {
        Pat3 black_pat = b.getPatternAt(p);
        double black_patWeight = gammaFunc(black_pat.toUint());
        if(b.bs(p) != BoardState::EMPTY()) {
            black_patWeight = 0;
        }

        black.weight_total -= black.weights[p];
        black.weights[p] = black_patWeight;
        black.weight_total += black_patWeight;

        Pat3 white_pat = black_pat.invert_colors();
        double white_patWeight = gammaFunc(white_pat.toUint());
        if(b.bs(p) != BoardState::EMPTY()) {
            white_patWeight = 0;
        }

        white.weight_total -= white.weights[p];
        white.weights[p] = white_patWeight;
        white.weight_total += white_patWeight;
    }

    virtual void movePlayed(Board& b, Move m) {
        //update our weights for dirty pat3s
        for(uint i=0; i<b.pat3dirty.size(); i++) {
            Point p = b.pat3dirty[i];
            updatePointWeights(b, p);
        }
        if(koPoint.isValid()) {
          updatePointWeights(b, koPoint);
        }
        koPoint = b.koPoint;
        if(koPoint.isValid()) {
          black.weight_total -= black.weights[koPoint];
          black.weights[koPoint] = 0;

          white.weight_total -= white.weights[koPoint];
          white.weights[koPoint] = 0;
        }
        //LOG("dirty pat3s: %d, weight total: %f", b.pat3dirty.size(), weight_total);
        b.pat3dirty.reset();

        assertGoodState(b);
    }

    double getWeight(BoardState c, Point p) {
        Weights& w = c == BoardState::WHITE() ? white : black;
        return w.weights[p];
    }

    Move getRandomMove(Board& b, BoardState c) {
        Weights& w = c == BoardState::WHITE() ? white : black;

        double r = genrand_res53() * w.weight_total;
        //LOG("weight_total: %.3f r: %.3f", weight_total, r);

        double sum = 0;
        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];
            sum += w.weights[p];
            if(sum > r) {
                return Move(c, p);
            }
        }

        return Move(c, Point::pass());
    }

};


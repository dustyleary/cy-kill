#pragma once

struct Gammas {
    double gammas[2<<20];
    void load(const char* filename);
    Gammas();
};
extern Gammas gGammas;

struct GammaPlayer : public RandomPlayerBase {
    Point getRandomMove(Board& b, BoardState c) {
        if(!b.emptyPoints.size()) {
            return Point::pass();
        }

        PointSet valid_moves;
        NatMap<Point, double> weights;

        memcpy(&valid_moves, &b.emptyPoints, sizeof(valid_moves));

        double weight_total = 0;
        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];
            if(!b.isValidMcgMove(c, p)) {
                valid_moves.remove(p);
                continue;
            }
            Pat3 pat = b.canonicalPatternAt<3>(c, p);

            double pw = gGammas.gammas[pat.toUint()];
            weights[p] = pw;
            weight_total += pw;
        }
        if(!valid_moves.size()) {
            return Point::pass();
        }

        double r = genrand_res53() * weight_total;

        Point p;
        for(uint i=0; i<valid_moves.size(); i++) {
            p = valid_moves[i];
            double pw = weights[p];
            if(r<pw) {
                break;
            }
            r -= pw;
        }
        ASSERT(p.isValid());
        return p;
    }

    Point playRandomMove(Board& b, BoardState c) {
        Point p = getRandomMove(b, c);
        b.playMoveAssumeLegal(c, p);
        return p;
    }
};


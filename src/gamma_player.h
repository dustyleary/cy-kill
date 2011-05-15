#pragma once

struct Gammas {
    double gammas[2<<20];
    void load(const char* filename);
    Gammas();
};
extern Gammas gGammas;

struct GammaPlayer : public RandomPlayerBase {
    NatMap<Point, double> weights;
    double weight_total;

    void resetStateForNewBoard(Board& b) {
        weight_total = 0;
        weights.setAll(0);

        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];
            Pat3 pat = b.getPatternAt(p);

            double patWeight = gGammas.gammas[pat.toUint()];
            weights[p] = patWeight;
            weight_total += patWeight;
        }
        //LOG("reset: %d empty points, weight total: %f", b.emptyPoints.size(), weight_total);
        b.pat3dirty.reset();
        assertNonZeroWeightsAreEmpty(b);
    }

    void assertNonZeroWeightsAreEmpty(Board& b) {
        if(!kCheckAsserts) return;
        FOREACH_NAT(Point, p, {
            double pw = weights[p];
            if(pw > 0) {
                ASSERT(b.bs(p) == BoardState::EMPTY());
            }
        });
    }

    Point getRandomMove(Board& b, BoardState c) {
        if(!b.emptyPoints.size()) {
            return Point::pass();
        }

        //update our weights for dirty pat3s
        for(uint i=0; i<b.pat3dirty.size(); i++) {
            Point p = b.pat3dirty[i];
            Pat3 pat = b.getPatternAt(p);
            double patWeight = gGammas.gammas[pat.toUint()];

            if(b.bs(p) != BoardState::EMPTY()) {
                patWeight = 0;
            }

            weight_total -= weights[p];
            weights[p] = patWeight;
            weight_total += patWeight;
        }
        //LOG("dirty pat3s: %d, weight total: %f", b.pat3dirty.size(), weight_total);
        b.pat3dirty.reset();

        assertNonZeroWeightsAreEmpty(b);

        double r = genrand_res53() * weight_total;
        //LOG("weight_total: %.3f r: %.3f", weight_total, r);

        double sum = 0;
        for(uint i=0; i<b.emptyPoints.size(); i++) {
            Point p = b.emptyPoints[i];
            sum += weights[p];
            if(sum >= r) {
                return p;
            }
        }

        return Point::pass();
    }

};


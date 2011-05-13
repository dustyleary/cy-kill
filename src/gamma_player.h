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

        Point foundMove = Point::invalid();

        double r = genrand_res53() * weight_total;

        //this is FOREACH_BOARD_POINT, just outside of Board
        for(uint y=0; y<b.getSize(); y++) {
            for(uint x=0; x<b.getSize(); x++) {
                Point p = COORD(x,y);
                double pw = weights[p];
                if(r<pw) {
                    foundMove = p;
                    break;
                }
                r -= pw;
            }
        }

        if(!foundMove.isValid()) {
            return Point::pass();
        }

        return foundMove;
    }

};


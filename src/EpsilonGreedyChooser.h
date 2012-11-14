#pragma once

class EpsilonGreedyChooser : public WeightedRandomChooser {
    double greed;

public:
    EpsilonGreedyChooser(double greed) : greed(greed) {
    }

    virtual ~EpsilonGreedyChooser() {}

    virtual int choose(uint weight_count, double* weights, double weights_sum) {
        if(genrand_res53() < greed) {
            double* max = std::max_element(weights, weights+weight_count);
            return max-weights;
        } else {
            return gen_rand64() % weight_count;
        }
    }
};


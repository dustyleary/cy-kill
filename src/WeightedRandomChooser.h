#pragma once

struct WeightedRandomChooser {
    static int choose(uint weight_count, double* weights, double weights_sum) {
        double v = genrand_res53() * weights_sum;
        double sum = 0;
        for(uint i=0; i<weight_count; i++) {
            sum += weights[i];
            if(sum > v) {
                return i;
            }
        }
        return -1;
    }
};


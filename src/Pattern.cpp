#include "config.h"

Pat3 invertedColorPat3s[PAT3_COUNT];
double pat3Gamma[PAT3_COUNT];

Pat3 getPat3InvertedColors(uint patternId) {
  return invertedColorPat3s[patternId];
}

double getPat3Gamma(Pat3 p) {
    return pat3Gamma[p.toUint()];
}

struct INIT {
    INIT() {
        for(uint patternId=0; patternId<PAT3_COUNT; patternId++) {
            Pat3 p = Pat3::fromUint(patternId);
            Pat3 inv = p._calculate_inverted_colors();
            invertedColorPat3s[patternId] = inv;

            pat3Gamma[patternId] = 1.0;
        }
    }
} gINIT;


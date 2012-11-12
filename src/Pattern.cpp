#include "config.h"

const uint PAT3_COUNT = 2<<20;
Pat3 invertedColorPat3s[PAT3_COUNT];

Pat3 getPat3InvertedColors(uint patternId) {
  return invertedColorPat3s[patternId];
}

struct INIT {
    INIT() {
        for(uint patternId=0; patternId<PAT3_COUNT; patternId++) {
            Pat3 p = Pat3::fromUint(patternId);
            Pat3 inv = p._calculate_inverted_colors();
            invertedColorPat3s[patternId] = inv;
        }
    }
} gINIT;


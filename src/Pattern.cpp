#include "config.h"

Pat3 invertedColorPat3s[PAT3_COUNT];
float pat3Gamma[PAT3_COUNT];

Pat3 getPat3InvertedColors(uint patternId) {
  return invertedColorPat3s[patternId];
}

float getPat3Gamma(Pat3 p) {
    return pat3Gamma[p.toUint()];
}

struct INIT {
    void calculateInvertedPat3s() {
        for(uint patternId=0; patternId<PAT3_COUNT; patternId++) {
            Pat3 p = Pat3::fromUint(patternId);
            Pat3 inv = p._calculate_inverted_colors();
            invertedColorPat3s[patternId] = inv;

            pat3Gamma[patternId] = 0.001;
        }
    }

    void loadPat3GammasFromFile() {
        FILE* fp = fopen("pat3_gammas.txt", "r");
        if(!fp) {
            fprintf(stderr, "could not open pat3_gammas.txt");
            exit(1);
        }

        char line[512];
        while(fgets(line, 512, fp)) {
            char pattext[512];
            double gamma;
            sscanf(line, "%s %lf", pattext, &gamma);

            Pat3 pat = Pat3::fromString(pattext);
            pat3Gamma[pat.toUint()] = gamma;
        }

        for(uint patternId=0; patternId<PAT3_COUNT; patternId++) {
            Pat3 p = Pat3::fromUint(patternId);
            pat3Gamma[p.toUint()] = pat3Gamma[p.canonical().toUint()];
        }
    }

    INIT() {
        LOG("calculateInvertedPat3s");
        calculateInvertedPat3s();
        LOG("calculateInvertedPat3s done");

        if(true) {
            LOG("loadPat3GammasFromFile");
            loadPat3GammasFromFile();
            LOG("loadPat3GammasFromFile done");
        }
    }
};

INIT gINIT;


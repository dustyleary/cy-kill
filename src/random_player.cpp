#include "config.h"

RandomPlayerPtr newRandomPlayer(const std::string& className) {
    if(false) {
    } else if(className == "PureRandomPlayer") {
        return RandomPlayerPtr(new PureRandomPlayer());
    } else if(className == "GammaPlayer") {
        return RandomPlayerPtr(new GammaPlayer());
    } else {
        return RandomPlayerPtr();
    }
}

RandomPlayerBase::~RandomPlayerBase() {
}


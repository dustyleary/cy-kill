#include "config.h"

std::string preprocess_line(std::string line) {
    //remove/replace bad chars
    std::string::iterator i = line.begin();
    while(i != line.end()) {
        if(*i == '\t' || *i == '\n') {
            *i++ = ' ';
            continue;
        }
        if(*i < 32 || *i == 127 ) {
            line.erase(i);
            continue;
        }
        if(*i == '#') {
            line = line.substr(0, i-line.begin());
            break;
        }
        ++i;
    }
    //remove starting whitespace
    size_t j = line.find_first_not_of(" ");
    if(j != std::string::npos) {
        line = line.substr(j);
    } else {
        return "";
    }

    //remove trailing whitespace
    j = line.find_last_not_of(" ");
    if(j != std::string::npos) {
        ++j;
    }
    line = line.substr(0, j);
    return line;
}

#include "config.h"

typedef std::vector<std::string> GtpArgs;

struct GtpCommand {
    uint id;
};

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    char line[1024];
    while(true) {
        char* c = fgets(line, 1024, stdin);
        if(!c) {
            if(feof(stdin)) {
                return 0;
            } else {
                fputs("error reading input\n", stderr);
                return 1;
            }
        }
        if(false) {
        } else if(0 == strcmp(line, "name")) {
            fputs("");
        }
    }

    return 0;
}

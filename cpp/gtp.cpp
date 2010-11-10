#include "config.h"

std::string trim(const std::string& in) {
    size_t s = in.find_first_not_of(" ");
    if(s == std::string::npos) { return ""; }
    size_t e = in.find_last_not_of(" ");
    if(e == std::string::npos) { e = in.size(); }
    return in.substr(s, e-s+1);
}

bool is_integer(const std::string& token) {
    for(std::string::const_iterator i = token.begin(); i!=token.end(); ++i) {
        if(!isdigit(*i)) return false;
    }
    return true;
}

int parse_integer(const std::string& token) {
    ASSERT(is_integer(token));
    return atoi(token.c_str());
}

std::string Gtp::preprocess_line(std::string line) {
    //remove/replace bad chars
    std::string::iterator i = line.begin();
    while(i != line.end()) {
        if(*i == '\t' || *i == '\n' || *i == '\r') {
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
    return trim(line);
}

bool Gtp::parse_line(const std::string& line2, GtpCommand& result) {
    std::string line = preprocess_line(line2);
    std::vector<std::string> tokens;
    size_t p = 0;
    while(true) {
        p = line.find_first_not_of(' ', p);
        if(p == std::string::npos) {
            break;
        }
        size_t e = line.find_first_of(' ', p);
        if(e == std::string::npos) {
            e = line.size();
        }
        tokens.push_back(trim(line.substr(p, e-p+1)));
        p = e;
    }

    if(tokens.empty()) return false;

    const std::string& token = tokens[0];
    if(is_integer(token)) {
        result.id = parse_integer(token);
        tokens.erase(tokens.begin());
    } else {
        result.id = -1;
    }

    if(tokens.empty()) return false;
    result.command = tokens[0];
    tokens.erase(tokens.begin());

    result.args = tokens;
    return true;
}

void Gtp::registerMethod(const std::string& cmd_name, MethodMap::mapped_type gcm) {
    command_methods[cmd_name] = gcm;
}

std::string Gtp::GtpSuccess(const std::string& msg) { return "= "+msg+"\n\n"; }
std::string Gtp::GtpFailure(const std::string& msg) { return "? "+msg+"\n\n"; }

std::string Gtp::protocol_version(const GtpCommand& gc) { return GtpSuccess("2"); }
std::string Gtp::name(const GtpCommand& gc) { return GtpSuccess("cy-kill"); }
std::string Gtp::version(const GtpCommand& gc) { return GtpSuccess("0.1"); }
std::string Gtp::quit(const GtpCommand& gc) { exit(0); }

std::string Gtp::known_command(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("could not parse argument");
    }
    std::string cmd_name = gc.args[0];
    MethodMap::iterator i1 = command_methods.find(cmd_name);
    if(i1 != command_methods.end()) {
        return GtpSuccess("true");
    }
    return GtpSuccess("false");
}

std::string Gtp::list_commands(const GtpCommand& gc) {
    std::string result;
    MethodMap::iterator i1 = command_methods.begin();
    while(i1 != command_methods.end()) {
        if(!result.empty()) result += "\n";
        result += (i1++)->first;
    }
    return GtpSuccess(result);
}

Gtp::Gtp() {
    registerMethod("protocol_version", &Gtp::protocol_version);
    registerMethod("name", &Gtp::name);
    registerMethod("version", &Gtp::version);
    registerMethod("known_command", &Gtp::known_command);
    registerMethod("list_commands", &Gtp::list_commands);
    registerMethod("quit", &Gtp::quit);
    //registerHandler("boardsize", );
    //registerHandler("clear_board", );
    //registerHandler("komi", );
    //registerHandler("play", );
    //registerHandler("genmove", );
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

std::string Gtp::run_cmd(const std::string& in) {
    GtpCommand gc;
    if(!parse_line(in, gc)) {
        return GtpFailure("parse error");
    }
    MethodMap::iterator i1 = command_methods.find(gc.command);
    if(i1 != command_methods.end()) {
        return CALL_MEMBER_FN(*this, i1->second)(gc);
    }
    return GtpFailure("unknown command");
}

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
    m_commandMethods[cmd_name] = gcm;
}

std::string Gtp::GtpSuccess() { return "=\n\n"; }
std::string Gtp::GtpSuccess(const std::string& msg) { return "= "+msg+"\n\n"; }
std::string Gtp::GtpFailure(const std::string& msg) { return "? "+msg+"\n\n"; }

std::string Gtp::protocol_version(const GtpCommand& gc) { return GtpSuccess("2"); }
std::string Gtp::name(const GtpCommand& gc) { return GtpSuccess("cy-kill"); }
std::string Gtp::version(const GtpCommand& gc) { return GtpSuccess("0.1"); }
std::string Gtp::quit(const GtpCommand& gc) { exit(0); }

std::string Gtp::known_command(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error");
    }
    std::string cmd_name = gc.args[0];
    MethodMap::iterator i1 = m_commandMethods.find(cmd_name);
    if(i1 != m_commandMethods.end()) {
        return GtpSuccess("true");
    }
    return GtpSuccess("false");
}

std::string Gtp::list_commands(const GtpCommand& gc) {
    std::string result;
    MethodMap::iterator i1 = m_commandMethods.begin();
    while(i1 != m_commandMethods.end()) {
        if(!result.empty()) result += "\n";
        result += (i1++)->first;
    }
    return GtpSuccess(result);
}

std::string Gtp::boardsize(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error");
    }
    if(!is_integer(gc.args[0])) {
        return GtpFailure("syntax error");
    }
    int i = parse_integer(gc.args[0]);
    switch(i) {
        case 9: m_boardSize = 9; break;
        case 19: m_boardSize = 19; break;
        default:
            return GtpFailure("unsupported board size");
    }
    m_boardSize = 19;
    clear_board(gc);
    return GtpSuccess();
}

std::string Gtp::clear_board(const GtpCommand& gc) {
    switch(m_boardSize) {
        case 9: m_board9.reset(); return GtpSuccess();
        case 19: m_board19.reset(); return GtpSuccess();
        default:
            ASSERT(false && "unhandled m_boardSize");
    }
    return GtpFailure("internal error");
}

std::string Gtp::komi(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error");
    }
    double k = atof(gc.args[0].c_str());
    if(k == 0.0) {
        return GtpFailure("syntax error");
    }
    m_komi = k;
    return GtpSuccess();
}

bool Gtp::parseGtpVertex(const std::string& in, std::pair<int,int>& out) {
    if(in.size() < 2) return false;
    char char1 = tolower(in[0]);
    std::string num = in.substr(1);
    if(!is_integer(num)) {
        return false;
    }
    int x = char1 - 'a';
    if(x > ('i' - 'a')) {
        x--;
    }
    int y = m_boardSize - parse_integer(num);
    out = std::pair<int,int>(x, y);
    return true;
}

bool Gtp::parseGtpColor(const std::string& in, BoardState& out) {
    if(in.empty()) return false;
    if(in[0] == 'w' || in[0] == 'W') {
        out = BoardState::WHITE();
        return true;
    }
    if(in[0] == 'b' || in[0] == 'B') {
        out = BoardState::BLACK();
        return true;
    }
    return false;
}

std::string Gtp::play(const GtpCommand& gc) {
    if(gc.args.size() != 2) {
        return GtpFailure("syntax error");
    }
    BoardState color;
    if(!parseGtpColor(gc.args[0], color)) {
        return GtpFailure("syntax error");
    }
    std::pair<int,int> vertex;
    if(!parseGtpVertex(gc.args[1], vertex)) {
        return GtpFailure("syntax error");
    }
    switch(m_boardSize) {
        case 9: {
            if(!m_board9.isValidMove(color, m_board9.COORD(vertex))) {
                return GtpFailure("illegal move");
            }
            m_board9.playMoveAssumeLegal(color, m_board9.COORD(vertex));
            break;
        }
        case 19: {
            if(!m_board19.isValidMove(color, m_board19.COORD(vertex))) {
                return GtpFailure("illegal move");
            }
            m_board19.playMoveAssumeLegal(color, m_board19.COORD(vertex));
            break;
        }

        default:
            ASSERT(false && "unhandled m_boardSize");
            return GtpFailure("internal error");
    }
    return GtpSuccess();
}

std::string Gtp::genmove(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error");
    }
    BoardState color;
    if(!parseGtpColor(gc.args[0], color)) {
        return GtpFailure("syntax error");
    }
    switch(m_boardSize) {
        case 9: {
            m_board9.playRandomMove(color);
            return GtpSuccess(m_board9.lastMove.toGtpVertex());
        }
        case 19: {
            m_board19.playRandomMove(color);
            return GtpSuccess(m_board19.lastMove.toGtpVertex());
        }
        default:
            ASSERT(false && "unhandled m_boardSize");
            return GtpFailure("internal error");
    }
}

Gtp::Gtp() {
    m_boardSize = 19;
    m_komi = 6.5f;
    clear_board(GtpCommand());

    registerMethod("protocol_version", &Gtp::protocol_version);
    registerMethod("name", &Gtp::name);
    registerMethod("version", &Gtp::version);
    registerMethod("known_command", &Gtp::known_command);
    registerMethod("list_commands", &Gtp::list_commands);
    registerMethod("quit", &Gtp::quit);
    registerMethod("boardsize", &Gtp::boardsize);
    registerMethod("clear_board", &Gtp::clear_board);
    registerMethod("komi", &Gtp::komi);
    registerMethod("play", &Gtp::play);
    registerMethod("genmove", &Gtp::genmove);
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

std::string Gtp::run_cmd(const std::string& in) {
    GtpCommand gc;
    if(!parse_line(in, gc)) {
        return GtpFailure("parse error");
    }
    MethodMap::iterator i1 = m_commandMethods.find(gc.command);
    if(i1 != m_commandMethods.end()) {
        return CALL_MEMBER_FN(*this, i1->second)(gc);
    }
    return GtpFailure("unknown command");
}
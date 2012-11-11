//
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

bool is_double(const std::string& token) {
    return (0 != atof(token.c_str()));
}

int parse_integer(const std::string& token) {
    ASSERT(is_integer(token));
    return atoi(token.c_str());
}

double parse_double(const std::string& token) {
    ASSERT(is_double(token));
    return atof(token.c_str());
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

    if(tokens.empty()) {
        result.command = "";
        return true;
    }

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

void Gtp::registerIntParam(uint* v, const std::string& label) {
    m_intParams[label] = v;
}

void Gtp::registerDoubleParam(double* v, const std::string& label) {
    m_doubleParams[label] = v;
}

std::string Gtp::GtpSuccess() { return "=\n\n"; }
std::string Gtp::GtpSuccess(const std::string& msg) { return "= "+msg+"\n\n"; }
std::string Gtp::GtpFailure(const std::string& msg, const GtpCommand& gc) {
    std::string cmdtext = gc.command;
    for(uint i=0; i<gc.args.size(); i++) {
        cmdtext += " "+gc.args[i];
    }
    return "? "+msg+strprintf(" # cmd: '%s'", cmdtext.c_str())+"\n\n";
}

std::string Gtp::protocol_version(const GtpCommand& gc) { return GtpSuccess("2"); }
std::string Gtp::name(const GtpCommand& gc) { return GtpSuccess("cy-kill"); }
std::string Gtp::version(const GtpCommand& gc) { return GtpSuccess("0.1"); }
std::string Gtp::quit(const GtpCommand& gc) {
    cykill_quit();
    return GtpSuccess();
}

std::string Gtp::gogui_interrupt(const GtpCommand& gc) {
    return GtpSuccess();
}

std::string Gtp::known_command(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error", gc);
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

std::string Gtp::engine_param(const GtpCommand& gc) {
    if(gc.args.size() == 0) {
        std::string result;
        IntParamMap::iterator i1 = m_intParams.begin();
        while(i1 != m_intParams.end()) {
            if(!result.empty()) result += "\n";
            result += strprintf("[string] %s %d", i1->first.c_str(), *i1->second);
            ++i1;
        }
        DoubleParamMap::iterator i2 = m_doubleParams.begin();
        while(i2 != m_doubleParams.end()) {
            if(!result.empty()) result += "\n";
            result += strprintf("[string] %s %f", i2->first.c_str(), *i2->second);
            ++i2;
        }
        return GtpSuccess(result);
    }
    if(gc.args.size() != 2) {
        return GtpFailure("syntax error", gc);
    }
    IntParamMap::iterator i1 = m_intParams.find(gc.args[0]);
    if(i1 != m_intParams.end()) {
        if(!is_integer(gc.args[1])) {
            return GtpFailure("integer required", gc);
        }
        *i1->second = parse_integer(gc.args[1]);
        return GtpSuccess();
    }
    DoubleParamMap::iterator i2 = m_doubleParams.find(gc.args[0]);
    if(i2 != m_doubleParams.end()) {
        if(!is_double(gc.args[1])) {
            return GtpFailure("double required", gc);
        }
        *i2->second = parse_double(gc.args[1]);
        return GtpSuccess();
    }

    return GtpFailure("unknown param", gc);
}

std::string Gtp::boardsize(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error", gc);
    }
    if(!is_integer(gc.args[0])) {
        return GtpFailure("syntax error", gc);
    }
    int i = parse_integer(gc.args[0]);
    if(i > kMaxBoardSize) {
        return GtpFailure("board size too large", gc);
    }
    m_board = Board(i, m_komi);
    return clear_board(gc);
}

std::string Gtp::buffer_io(const GtpCommand& gc) {
    int do_buffer = 1;
    if(gc.args.size() == 1) {
        if(!is_integer(gc.args[0])) {
            return GtpFailure("syntax error", gc);
        }
        do_buffer = parse_integer(gc.args[0]);
    }
    if(do_buffer) {
        if(fout) setbuf(fout, (char*)malloc(BUFSIZ));
        if(ferr) setbuf(ferr, (char*)malloc(BUFSIZ));
    } else {
        if(fout) setbuf(fout, NULL);
        if(ferr) setbuf(ferr, NULL);
    }
    return GtpSuccess();
}

std::string Gtp::final_score(const GtpCommand& gc) {
    double score = m_board.trompTaylorScore();
    if(score > 0) {
      return GtpSuccess(strprintf("W+%.1f", score));
    } else {
      return GtpSuccess(strprintf("B+%.1f", -score));
    }
}

std::string Gtp::valid_move_patterns(const GtpCommand& gc) {
  if(gc.args.size() != 2) {
      return GtpFailure("syntax error", gc);
  }
  PointColor color;
  if(!parseGtpColor(gc.args[0], color)) {
      return GtpFailure("syntax error", gc);
  }
  Point vertex;
  if(!parseGtpVertex(gc.args[1], vertex)) {
      return GtpFailure("syntax error", gc);
  }

  std::string result = "{";
  result += strprintf("\"winner\":\"%s\"", vertex.toGtpVertex(m_board.getSize()).c_str());
  result += ", \"fighters\": {";
  std::map<Point, Pattern<3> > movePatterns = m_board.getCanonicalPatternsForValidMoves<3>(color);
  std::map<Point, Pattern<3> >::iterator i1 = movePatterns.begin();
  int c = 0;
  while(i1 != movePatterns.end()) {
    const Point& pt = i1->first;
    const Pattern<3>& pat = i1->second;
    ++i1;
    if(pt == Point::pass()) {
      continue;
    }
    if(c != 0) {
      result += ", ";
    }
    result += strprintf("\"%s\":\"%s\"",
      pt.toGtpVertex(m_board.getSize()).c_str(),
      pat.toString().c_str()
    );
    c++;
  }
  result += "}}";

  return GtpSuccess(strprintf("# PATTERN_SHOWDOWN 3 %s", result.c_str()));
}

std::string Gtp::clear_board(const GtpCommand& gc) {
    uint seed = m_random_seed;
    if(seed == 0) {
      seed = cykill_millisTime();
    }
    LOG("using random seed: %d", seed);
    init_gen_rand(seed);
    m_board = Board(m_board.getSize(), m_komi);
    return GtpSuccess();
}

std::string Gtp::dump_board(const GtpCommand& gc) {
    m_board.dump();
    return GtpSuccess();
}

std::string Gtp::echo_text(const GtpCommand& gc) {
    std::string result = "";
    for(uint i=0; i<gc.args.size(); i++) {
        if(!result.empty()) result += " ";
        result += gc.args[i];
    }
    return GtpSuccess(result);
}

std::string Gtp::komi(const GtpCommand& gc) {
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error", gc);
    }
    double k = atof(gc.args[0].c_str());
    if(gc.args[0] == "0") {
        k = 0;
    } else if(k == 0.0) {
        return GtpFailure("syntax error", gc);
    }
    m_komi = (float)k;
    return clear_board(gc);
}

bool Gtp::parseGtpVertex(const std::string& in, Point& out) {
    if(in == "pass" || in == "PASS") {
        out = Point::pass();
        return true;
    }
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
    int y = m_board.getSize() - parse_integer(num);
    out = COORD(x, y);
    return true;
}

bool Gtp::parseGtpColor(const std::string& in, PointColor& out) {
    if(in.empty()) return false;
    if(in[0] == 'w' || in[0] == 'W') {
        out = PointColor::WHITE();
        return true;
    }
    if(in[0] == 'b' || in[0] == 'B') {
        out = PointColor::BLACK();
        return true;
    }
    return false;
}

std::string Gtp::play(const GtpCommand& gc) {
    if(gc.args.size() != 2) {
        return GtpFailure("syntax error", gc);
    }
    PointColor color;
    if(!parseGtpColor(gc.args[0], color)) {
        return GtpFailure("syntax error", gc);
    }
    Point vertex;
    if(!parseGtpVertex(gc.args[1], vertex)) {
        return GtpFailure("syntax error", gc);
    }
    Move m(color, vertex);
    if(!m_board.isValidMove(m)) {
        return GtpFailure("illegal move", gc);
    }
    m_board.playMoveAssumeLegal(m);
    return GtpSuccess();
}

std::string Gtp::genmove(const GtpCommand& gc) {
    fprintf(stderr, "gogui-gfx: CLEAR\n");
    if(gc.args.size() != 1) {
        return GtpFailure("syntax error", gc);
    }
    PointColor color;
    if(!parseGtpColor(gc.args[0], color)) {
        return GtpFailure("syntax error", gc);
    }

    Mcts2<Board> mcts;

    mcts.kTracesPerGuiUpdate = uct_kTracesPerGuiUpdate;
    mcts.kGuiShowMoves = uct_kGuiShowMoves;
    mcts.kUctC = uct_kUctC;
    mcts.kRaveEquivalentPlayouts = uct_kRaveEquivalentPlayouts;
    mcts.kMinVisitsForCertainty = uct_kMinVisitsForCertainty;
    mcts.kCountdownToCertainty = uct_kCountdownToCertainty;
    mcts.kNumPlayoutsPerTrace = uct_kNumPlayoutsPerTrace;

    mcts.kModuloPlayoutsNumerator = uct_kModuloPlayoutsNumerator;
    mcts.kModuloPlayoutsDenominator = uct_kModuloPlayoutsDenominator;

    RandomPlayerPtr randomPlayer = newRandomPlayer("PureRandomPlayer");

    uint32_t st = cykill_millisTime();
    uint32_t et = cykill_millisTime();
    while(true) {
      et = cykill_millisTime();
      if((et-st) > max_think_millis) {
        break;
      }
      if(mcts.total_playouts > max_playouts) {
        break;
      }
      if(mcts.gotMoveCertainty >= 1) {
        break;
      }
      if(needs_interrupt()) {
        clear_interrupt();
        break;
      }
      mcts.step(m_board, color, randomPlayer);
    }

    Move bestMove = mcts.getBestMove(m_board, color);
    m_board.playMoveAssumeLegal(bestMove);
    fprintf(stderr, "# total time: %.2f", (et-st)/1000.0);
    return GtpSuccess(bestMove.point.toGtpVertex(m_board.getSize()));
}

std::string Gtp::pattern_at(const GtpCommand& gc) {
    if(gc.args.size() != 3) {
        return GtpFailure("syntax error", gc);
    }
    PointColor color;
    if(!parseGtpColor(gc.args[0], color)) {
        return GtpFailure("syntax error", gc);
    }
    Point vertex;
    if(!parseGtpVertex(gc.args[1], vertex)) {
        return GtpFailure("syntax error", gc);
    }
    if(!is_integer(gc.args[2])) {
        return GtpFailure("syntax error", gc);
    }
    int size = parse_integer(gc.args[2]);

    if(vertex == Point::pass()) {
        return GtpSuccess("pass");
    }

    if(m_board.bs(vertex).isPlayer()) {
        return GtpSuccess("not-empty");
    }

#define doit(N) \
    case N: { \
        Pattern<N> p = m_board.canonicalPatternAt<N>(color, vertex); \
        return GtpSuccess(p.toString()); \
    }
    switch(size) {
        doit(3)
        doit(5)
        doit(7)
        doit(9)
        doit(11)
        doit(13)
        doit(15)
        doit(19)
    }
#undef doit
    return GtpFailure("unhandled size", gc);
}

volatile bool Gtp::needs_interrupt() {
    return _needs_interrupt;
}

void Gtp::clear_interrupt() {
    _needs_interrupt = false;
}

Gtp::Gtp(FILE* fin, FILE* fout, FILE* ferr)
    : m_komi(6.5f), m_board(19, m_komi), fin(fin), fout(fout), ferr(ferr)
    , _needs_interrupt(false)
{
    if(fout) setbuf(fout, NULL);
    if(ferr) setbuf(ferr, NULL);

    m_random_seed = 0;
    max_think_millis = 1000 * 60 * 60;
    max_playouts = 100000000;

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
    registerMethod("gogui-analyze_commands", &Gtp::gogui_analyze_commands);
    registerMethod("engine_param", &Gtp::engine_param);
    registerMethod("pattern_at", &Gtp::pattern_at);
    registerMethod("gogui-interrupt", &Gtp::gogui_interrupt);
    registerMethod("dump_board", &Gtp::dump_board);
    registerMethod("echo_text", &Gtp::echo_text);
    registerMethod("buffer_io", &Gtp::buffer_io);
    registerMethod("final_score", &Gtp::final_score);
    registerMethod("valid_move_patterns", &Gtp::valid_move_patterns);

    registerIntParam(&max_think_millis, "max_think_millis");
    registerIntParam(&max_playouts, "max_playouts");
    registerIntParam(&m_random_seed, "random_seed");

    uct_kTracesPerGuiUpdate = 5000;
    uct_kGuiShowMoves = 5;
    uct_kUctC = sqrt(2.0);
    uct_kRaveEquivalentPlayouts = 100;
    uct_kMinVisitsForCertainty = 3000;
    uct_kCountdownToCertainty = 100000;
    uct_kNumPlayoutsPerTrace = 3;

    uct_kModuloPlayoutsNumerator = 0;
    uct_kModuloPlayoutsDenominator = 1;

    registerIntParam(&uct_kTracesPerGuiUpdate, "uct_kTracesPerGuiUpdate");
    registerIntParam(&uct_kGuiShowMoves, "uct_kGuiShowMoves");
    registerDoubleParam(&uct_kUctC, "uct_kUctC");
    registerDoubleParam(&uct_kRaveEquivalentPlayouts, "uct_kRaveEquivalentPlayouts");
    registerIntParam(&uct_kMinVisitsForCertainty, "uct_kMinVisitsForCertainty");
    registerIntParam(&uct_kCountdownToCertainty, "uct_kCountdownToCertainty");
    registerIntParam(&uct_kNumPlayoutsPerTrace, "uct_kNumPlayoutsPerTrace");

    registerIntParam(&uct_kModuloPlayoutsNumerator, "uct_kModuloPlayoutsNumerator");
    registerIntParam(&uct_kModuloPlayoutsDenominator, "uct_kModuloPlayoutsDenominator");

    clear_board(GtpCommand());
}

std::string Gtp::gogui_analyze_commands(const GtpCommand& gc) {
    return GtpSuccess("param/Engine Params/engine_param");
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

std::string Gtp::run_cmd(const std::string& in) {
    GtpCommand gc;
    if(!parse_line(in, gc)) {
        return GtpFailure("parse error", gc);
    }
    if(gc.command.empty()) {
        return "";
    }
    MethodMap::iterator i1 = m_commandMethods.find(gc.command);
    if(i1 != m_commandMethods.end()) {
        return CALL_MEMBER_FN(*this, i1->second)(gc);
    }
    return GtpFailure("unknown command", gc);
}

void static_input_thread(void* v) {
    Gtp* gtp = (Gtp*)v;
    gtp->input_thread();
}

void Gtp::input_thread() {
    char inbuf[4096];

    while(true) {
#if 0
        input_mutex.acquire();
        if(lines.size() > 100000) {
            input_mutex.release();
            cykill_sleep(1);
            continue;
        }
        input_mutex.release();
#endif

        char* r = fgets(inbuf, sizeof(inbuf)-1, fin);
        if(!r) {
          if(feof(fin)) {
            sprintf(inbuf, "\nquit\n");
          } else {
            fprintf(ferr, "ERROR: error reading input: %d\n", ferror(fin));
            return;
          }
        }
        if(strstr(inbuf, "# interrupt")) {
            fprintf(ferr, "WANT-INTERRUPT\n");
            _needs_interrupt = true;
        } else {
            std::string line = inbuf;
            input_mutex.acquire();
            input_lines.push_back(line);
            input_mutex.release();
        }
        if(feof(fin)) {
          return;
        }
    }
}

void Gtp::run(int argc, char** argv) {
    for(int i=1; i<argc; i++) {
      std::string result = run_cmd(std::string(argv[i]));
      if(result[0] == '?') {
        fputs(result.c_str(), ferr);
        exit(1);
      }
    }
    cykill_startthread(static_input_thread, (void*)this);
    while(true) {
        input_mutex.acquire();
        if(input_lines.empty()) {
            input_mutex.release();
            cykill_sleep(1);
            continue;
        }
        std::list<std::string> new_lines;
        std::swap(new_lines, input_lines);
        input_mutex.release();

        for(std::list<std::string>::iterator i = new_lines.begin(); i!=new_lines.end(); ++i) {
            std::string result = run_cmd(*i);
            fputs(result.c_str(), fout);
        }
    }
}

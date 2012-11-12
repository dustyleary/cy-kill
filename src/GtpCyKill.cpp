#include "config.h"

bool GtpCyKill::parseGtpVertex(const std::string& in, Point& out) {
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

bool GtpCyKill::parseGtpColor(const std::string& in, PointColor& out) {
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

std::string GtpCyKill::boardsize(const GtpCommand& gc) {
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

std::string GtpCyKill::clear_board(const GtpCommand& gc) {
    uint seed = m_random_seed;
    if(seed == 0) {
      seed = cykill_millisTime();
    }
    LOG("using random seed: %d", seed);
    init_gen_rand(seed);
    m_board = Board(m_board.getSize(), m_komi);
    return GtpSuccess();
}

std::string GtpCyKill::dump_board(const GtpCommand& gc) {
    m_board.dump();
    return GtpSuccess();
}

std::string GtpCyKill::final_score(const GtpCommand& gc) {
    double score = m_board.trompTaylorScore();
    if(score > 0) {
      return GtpSuccess(strprintf("W+%.1f", score));
    } else {
      return GtpSuccess(strprintf("B+%.1f", -score));
    }
}

std::string GtpCyKill::genmove(const GtpCommand& gc) {
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
      mcts.step(m_board, color);
    }

    Move bestMove = mcts.getBestMove(m_board, color);
    m_board.playMoveAssumeLegal(bestMove);
    fprintf(stderr, "# total time: %.2f", (et-st)/1000.0);
    return GtpSuccess(bestMove.point.toGtpVertex(m_board.getSize()));
}

std::string GtpCyKill::komi(const GtpCommand& gc) {
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

std::string GtpCyKill::pattern_at(const GtpCommand& gc) {
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

std::string GtpCyKill::play(const GtpCommand& gc) {
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

std::string GtpCyKill::valid_move_patterns(const GtpCommand& gc) {
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

GtpCyKill::GtpCyKill(FILE* fin, FILE* fout, FILE* ferr)
    : Gtp(fin,fout,ferr)
    , m_komi(6.5f), m_board(19, m_komi) 
{
    registerMethod("boardsize", &GtpCyKill::boardsize);
    registerMethod("clear_board", &GtpCyKill::clear_board);
    registerMethod("dump_board", &GtpCyKill::dump_board);
    registerMethod("final_score", &GtpCyKill::final_score);
    registerMethod("genmove", &GtpCyKill::genmove);
    registerMethod("komi", &GtpCyKill::komi);
    registerMethod("pattern_at", &GtpCyKill::pattern_at);
    registerMethod("play", &GtpCyKill::play);
    registerMethod("valid_move_patterns", &GtpCyKill::valid_move_patterns);

    m_random_seed = 0;
    max_think_millis = 1000 * 60 * 60;
    max_playouts = 100000000;

    registerIntParam(&m_random_seed, "random_seed");
    registerIntParam(&max_playouts, "max_playouts");
    registerIntParam(&max_think_millis, "max_think_millis");

    uct_kRaveEquivalentPlayouts = 100;
    uct_kUctC = sqrt(2.0);
    uct_kCountdownToCertainty = 100000;
    uct_kGuiShowMoves = 5;
    uct_kMinVisitsForCertainty = 3000;
    uct_kModuloPlayoutsDenominator = 1;
    uct_kModuloPlayoutsNumerator = 0;
    uct_kNumPlayoutsPerTrace = 3;
    uct_kTracesPerGuiUpdate = 5000;

    registerDoubleParam(&uct_kRaveEquivalentPlayouts, "uct_kRaveEquivalentPlayouts");
    registerDoubleParam(&uct_kUctC, "uct_kUctC");
    registerIntParam(&uct_kCountdownToCertainty, "uct_kCountdownToCertainty");
    registerIntParam(&uct_kGuiShowMoves, "uct_kGuiShowMoves");
    registerIntParam(&uct_kMinVisitsForCertainty, "uct_kMinVisitsForCertainty");
    registerIntParam(&uct_kModuloPlayoutsDenominator, "uct_kModuloPlayoutsDenominator");
    registerIntParam(&uct_kModuloPlayoutsNumerator, "uct_kModuloPlayoutsNumerator");
    registerIntParam(&uct_kNumPlayoutsPerTrace, "uct_kNumPlayoutsPerTrace");
    registerIntParam(&uct_kTracesPerGuiUpdate, "uct_kTracesPerGuiUpdate");

    clear_board(GtpCommand());
}


#include "config.h"

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

std::string GtpCyKill::final_score(const GtpCommand& gc) {
    double score = m_board.trompTaylorScore();
    if(score > 0) {
      return GtpSuccess(strprintf("W+%.1f", score));
    } else {
      return GtpSuccess(strprintf("B+%.1f", -score));
    }
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
    int size;
    PointColor color;
    Point vertex;

    if(gc.args.size() != 3) { return GtpFailure("syntax error", gc); }

    if(!is_integer(gc.args[0])) { return GtpFailure("syntax error", gc); }
    size = parse_integer(gc.args[0]);

    if(!parseGtpColor(gc.args[1], color)) { return GtpFailure("syntax error", gc); }
    if(!parseGtpVertex(gc.args[2], vertex)) { return GtpFailure("syntax error", gc); }

    if(vertex == Point::pass()) { return GtpSuccess("pass"); }

    if(m_board.bs(vertex).isPlayer()) {
        return GtpSuccess("not-empty");
    }

#define doit(N) \
    case N: { \
        Pattern<N> p = m_board.canonicalPatternAt<N>(color, vertex); \
        p.dump(); \
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

std::string GtpCyKill::valid_move_patterns(const GtpCommand& gc) {
  int size;
  PointColor color;

  if(gc.args.size() != 2) { return GtpFailure("syntax error", gc); }

  if(!is_integer(gc.args[0])) { return GtpFailure("syntax error", gc); }
  size = parse_integer(gc.args[0]);

  if(!parseGtpColor(gc.args[1], color)) { return GtpFailure("syntax error", gc); }

#define doit(N) case N: { return _internal_valid_move_patterns<N>(color); }
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

}

template<int SIZE>
std::string GtpCyKill::_internal_valid_move_patterns(PointColor color) {
  std::string result = "{";
  std::map<Point, Pattern<SIZE> > movePatterns = m_board.getCanonicalPatternsForValidMoves<SIZE>(color);
  typename std::map<Point, Pattern<SIZE> >::iterator i1 = movePatterns.begin();
  int c = 0;
  while(i1 != movePatterns.end()) {
    const Point& pt = i1->first;
    const Pattern<SIZE>& pat = i1->second;
    ++i1;
    if(pt == Point::pass()) {
      continue;
    }
    if(c != 0) {
      result += ", ";
    }
    result += strprintf("\"%s\":\"%s\"",
      pt.toGtpVertex().c_str(),
      pat.toString().c_str()
    );
    c++;
  }
  result += "}";

  return GtpSuccess(strprintf("# VALID_MOVE_PATTERNS %d %s", SIZE, result.c_str()));
}

GtpCyKill::GtpCyKill(FILE* fin, FILE* fout, FILE* ferr)
    : GtpMcts<Board>(fin,fout,ferr)
    , m_komi(6.5f)
{
    m_board = Board(19, m_komi);

    registerMethod("boardsize", &GtpCyKill::boardsize);
    registerMethod("final_score", &GtpCyKill::final_score);
    registerMethod("komi", &GtpCyKill::komi);
    registerMethod("pattern_at", &GtpCyKill::pattern_at);
    registerMethod("valid_move_patterns", &GtpCyKill::valid_move_patterns);

    uct_kCountdownToCertainty = 1000 * 1000 * 1000;
    max_traces = 1 * 1000 * 1000;

    clear_board(GtpCommand());
}


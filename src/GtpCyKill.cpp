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

    if(vertex == Point::pass()) { return GtpFailure("expected real vertex, got 'pass'", gc); }

#define doit(N) \
    case N: { \
        Pattern<N> p = m_board.canonicalPatternAt<N>(color, vertex); \
        return GtpSuccess(std::string("PATTERN_AT_RESULT: ")+p.toString()); \
    }
    switch(size) {
        case 3: {
            Pat3 p = m_board.canonicalPatternAt<3>(color, vertex);
            LOG("pattern: %s gamma: %.6f cachedGamma: %.6f", p.toString().c_str(), getPat3Gamma(p), m_board.cachedGammaAt(color, vertex));
            return GtpSuccess(std::string("PATTERN_AT_RESULT: ")+p.toString());
        }
        doit(5)
        doit(7)
        doit(9)
        doit(11)
        doit(13)
        doit(15)
        doit(17)
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
        doit(17)
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

  return GtpSuccess(strprintf("VALID_MOVE_PATTERNS %d %s", SIZE, result.c_str()));
}

std::string GtpCyKill::show_current_tromp_taylor(const GtpCommand& gc) {
  PointSet black, white;
  m_board.trompTaylorOwners(black, white);

  std::string gfx = "CLEAR\n";
  gfx += "BLACK";
  for(uint i=0; i<black.size(); i++) {
    gfx += ' ';
    gfx += black[i].toGtpVertex();
  }
  gfx += '\n';

  gfx += "WHITE";
  for(uint i=0; i<white.size(); i++) {
    gfx += ' ';
    gfx += white[i].toGtpVertex();
  }
  gfx += '\n';

  gfx += strprintf("TEXT Komi: %.1f  Tromp-Taylor score: %.1f", m_board.komi, m_board.trompTaylorScore());

  return GtpSuccess(gfx);
}

std::string GtpCyKill::monte_carlo_score_estimate(const GtpCommand& gc) {
  if(gc.args.size() == 0) return GtpSuccess("10000");
  if(gc.args.size() != 1) return GtpFailure("syntax error", gc);

  if(!is_integer(gc.args[0])) { return GtpFailure("syntax error", gc); }
  int playouts = parse_integer(gc.args[0]);
  if(playouts < 1) { return GtpFailure("syntax error", gc); }

  NatMap<Point, int> black_owned(0);
  NatMap<Point, int> white_owned(0);

  PointColor color = m_board.getWhosTurn();

  double unsure_threshold = double(playouts) * m_MonteCarloScoreEstimate_unsure_fraction;
  Board copy;
  for(int p=0; p<playouts; p++) {
    copy = m_board;
    PlayoutResults pr;
    doRandomPlayout(copy, color, pr);
    PointSet black, white;
    copy.trompTaylorOwners(black, white);
    for(uint y=0; y<copy.getSize(); y++) {
      for(uint x=0; x<copy.getSize(); x++) {
        Point p = COORD(x,y);
        if(black.contains(p)) black_owned[p]++;
        if(white.contains(p)) white_owned[p]++;
      }
    }
  }

  for(uint y=0; y<m_board.getSize(); y++) {
    for(uint x=0; x<m_board.getSize(); x++) {
      Point p = COORD(x,y);
      LOG("%3s %6d %6d %6d", p.toGtpVertex().c_str(), black_owned[p], white_owned[p], black_owned[p]+white_owned[p]);
    }
  }
  copy.dump();

  std::string gfx = "CLEAR\n";

  uint black_points = 0;
  uint white_points = 0;
  gfx += "BLACK";
  for(uint y=0; y<m_board.getSize(); y++) {
    for(uint x=0; x<m_board.getSize(); x++) {
      Point p = COORD(x,y);
      if((black_owned[p] - white_owned[p]) >= unsure_threshold) {
        gfx += ' ';
        gfx += p.toGtpVertex();
        black_points++;
      }
    }
  }
  gfx += '\n';

  gfx += "WHITE";
  for(uint y=0; y<m_board.getSize(); y++) {
    for(uint x=0; x<m_board.getSize(); x++) {
      Point p = COORD(x,y);
      if((white_owned[p] - black_owned[p]) >= unsure_threshold) {
        gfx += ' ';
        gfx += p.toGtpVertex();
        white_points++;
      }
    }
  }
  gfx += '\n';
  double unsure_points = m_board.getSize() * m_board.getSize() - black_points - white_points;

  double ttScore = m_board.komi + white_points - black_points + unsure_points/2;
  gfx += strprintf("TEXT Komi: %.1f  Tromp-Taylor score: %.1f +/- %.1f", m_board.komi, ttScore, unsure_points/2);

  return GtpSuccess(gfx);
}

std::string GtpCyKill::show_current_pat3_gammas(const GtpCommand& gc) {
  if(gc.args.size() != 0) return GtpFailure("syntax error", gc);

  PointColor color = m_board.getWhosTurn();

  std::string gfx = "CLEAR\n";
  gfx += "LABEL";
  for(uint i=0; i<m_board.emptyPoints.size(); i++) { 
      Point p = m_board.emptyPoints[i];
      if(m_board.isValidMove(color, p)) {
          gfx += strprintf(" %s %.2f", p.toGtpVertex().c_str(), m_board.cachedGammaAt(color, p));
      }
  }
  gfx += '\n';

  return GtpSuccess(gfx);
}

GtpCyKill::GtpCyKill(FILE* fin, FILE* fout, FILE* ferr)
    : GtpMcts<Board>(fin,fout,ferr)
    , m_komi(6.5f)
    , m_MonteCarloScoreEstimate_unsure_fraction(0.1f)
{
    registerMethod("boardsize", &GtpCyKill::boardsize);
    registerMethod("final_score", &GtpCyKill::final_score);
    registerMethod("komi", &GtpCyKill::komi);
    registerMethod("pattern_at", &GtpCyKill::pattern_at);
    registerMethod("valid_move_patterns", &GtpCyKill::valid_move_patterns);
    registerMethod("show_current_tromp_taylor", &GtpCyKill::show_current_tromp_taylor);
    registerMethod("monte_carlo_score_estimate", &GtpCyKill::monte_carlo_score_estimate);
    registerMethod("show_current_pat3_gammas", &GtpCyKill::show_current_pat3_gammas);

    registerAnalyzeCommand("gfx/Show Current Tromp-Taylor ownership/show_current_tromp_taylor");
    registerAnalyzeCommand("gfx/Monte Carlo Score Estimate/monte_carlo_score_estimate %s");
    registerAnalyzeCommand("gfx/Show Pat3 Gammas/show_current_pat3_gammas");

    registerDoubleParam(&m_MonteCarloScoreEstimate_unsure_fraction, "MonteCarloScoreEstimate_unsure_fraction");

    setOpeningBook(boost::shared_ptr<OpeningBook<Board> >(new MysqlOpeningBook()));

    uct_kCountdownToCertainty = 1000000;
    max_traces = 5 * 1000 * 1000;
    max_think_millis = 1000 * 60 * 5;

    m_board = Board(19, m_komi);
    //clear_board(GtpCommand());
}


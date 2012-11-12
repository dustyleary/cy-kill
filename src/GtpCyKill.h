#pragma once

class GtpCyKill : public Gtp {
public:
    GtpCyKill(FILE* fin=stdin, FILE* fout=stdout, FILE* ferr=stderr);

    static bool parseGtpColor(const std::string& in, PointColor& out);
    bool parseGtpVertex(const std::string& in, Point& out);

    std::string boardsize(const GtpCommand& gc);
    std::string clear_board(const GtpCommand& gc);
    std::string dump_board(const GtpCommand& gc);
    std::string final_score(const GtpCommand& gc);
    std::string genmove(const GtpCommand& gc);
    std::string komi(const GtpCommand& gc);
    std::string pattern_at(const GtpCommand& gc);
    std::string play(const GtpCommand& gc);
    std::string valid_move_patterns(const GtpCommand& gc);

private:
    float m_komi;
    Board m_board;

    uint m_random_seed;
    uint max_playouts;
    uint max_think_millis;

    //mcts
    double uct_kRaveEquivalentPlayouts;
    double uct_kUctC;
    uint uct_kCountdownToCertainty;
    uint uct_kGuiShowMoves;
    uint uct_kMinVisitsForCertainty;
    uint uct_kModuloPlayoutsDenominator;
    uint uct_kModuloPlayoutsNumerator;
    uint uct_kNumPlayoutsPerTrace;
    uint uct_kTracesPerGuiUpdate;
};


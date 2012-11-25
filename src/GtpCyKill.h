#pragma once

class GtpCyKill : public GtpMcts<Board> {
public:
    GtpCyKill(FILE* fin=stdin, FILE* fout=stdout, FILE* ferr=stderr);

    std::string boardsize(const GtpCommand& gc);
    std::string final_score(const GtpCommand& gc);
    std::string komi(const GtpCommand& gc);
    std::string pattern_at(const GtpCommand& gc);
    std::string valid_move_patterns(const GtpCommand& gc);
    std::string show_current_tromp_taylor(const GtpCommand& gc);
    std::string monte_carlo_score_estimate(const GtpCommand& gc);
    std::string show_current_pat3_gammas(const GtpCommand& gc);

private:
    template<int SIZE>
    std::string _internal_valid_move_patterns(PointColor color);

    float m_komi;
    double m_MonteCarloScoreEstimate_unsure_fraction;
};


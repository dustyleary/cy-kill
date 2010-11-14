#pragma once


struct GtpCommand {
    int id;
    std::string command;
    std::vector<std::string> args;
    GtpCommand() : id(-1) {}
};

class Gtp {
public:
    Gtp(FILE* fin=0, FILE* fout=0, FILE* ferr=0);

    static std::string preprocess_line(std::string line);
    static bool parse_line(const std::string& line, GtpCommand& result);

    static bool parseGtpColor(const std::string& in, BoardState& out);
    bool parseGtpVertex(const std::string& in, Point& out);

    static std::string GtpSuccess();
    static std::string GtpSuccess(const std::string& msg);
    static std::string GtpFailure(const std::string& msg, const GtpCommand& gc);

    std::string run_cmd(const std::string& in);
    void run();

    typedef std::string (Gtp::*GtpCommandMethod)(const GtpCommand& in);

    typedef std::map<std::string, GtpCommandMethod> MethodMap;
    MethodMap m_commandMethods;
    void registerMethod(const std::string& cmd_name, MethodMap::mapped_type gcm);

    typedef std::map<std::string, int*> IntParamMap;
    IntParamMap m_intParams;
    void registerIntParam(int* v, const std::string& label);

    typedef std::map<std::string, double*> DoubleParamMap;
    DoubleParamMap m_doubleParams;
    void registerDoubleParam(double* v, const std::string& label);

    std::string protocol_version(const GtpCommand& gc);
    std::string name(const GtpCommand& gc);
    std::string version(const GtpCommand& gc);
    std::string quit(const GtpCommand& gc);

    std::string known_command(const GtpCommand& gc);
    std::string list_commands(const GtpCommand& gc);
    std::string boardsize(const GtpCommand& gc);
    std::string clear_board(const GtpCommand& gc);
    std::string komi(const GtpCommand& gc);
    std::string play(const GtpCommand& gc);
    std::string genmove(const GtpCommand& gc);
    std::string gogui_analyze_commands(const GtpCommand& gc);
    std::string engine_param(const GtpCommand& gc);
    std::string pattern_at(const GtpCommand& gc);
    std::string gogui_interrupt(const GtpCommand& gc);
    std::string dump_board(const GtpCommand& gc);
    std::string echo_text(const GtpCommand& gc);
    std::string buffer_io(const GtpCommand& gc);

    void input_thread();

    volatile bool needs_interrupt();
    void clear_interrupt();

private:
    double m_komi;
    Board m_board;
    volatile bool _needs_interrupt;

    int m_monte_1ply_playouts_per_move;
    int uct_kPlayouts; //11
    int uct_kExpandThreshold; //5
    int uct_kStepSize; //100
    double uct_kUctK; //1.0
    int max_think_millis;

    double getMoveValue(BoardState color, Point p);

    FILE* fin;
    FILE* fout;
    FILE* ferr;
    std::list<std::string> lines;
    Mutex input_mutex;
};


#pragma once

struct GtpCommand {
    int id;
    std::string command;
    std::vector<std::string> args;
    GtpCommand() : id(-1) {}
};

class Gtp {
public:
    Gtp();

    static std::string preprocess_line(std::string line);
    static bool parse_line(const std::string& line, GtpCommand& result);

    static bool Gtp::parseGtpColor(const std::string& in, BoardState& out);
    bool Gtp::parseGtpVertex(const std::string& in, std::pair<int,int>& out);

    static std::string GtpSuccess();
    static std::string GtpSuccess(const std::string& msg);
    static std::string GtpFailure(const std::string& msg);

    std::string run_cmd(const std::string& in);

    typedef std::string (Gtp::*GtpCommandMethod)(const GtpCommand& in);

    typedef std::map<std::string, GtpCommandMethod> MethodMap;
    MethodMap m_commandMethods;

    void registerMethod(const std::string& cmd_name, MethodMap::mapped_type gcm);

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

    int m_boardSize;
    double m_komi;
    Board<9> m_board9;
    Board<19> m_board19;
};


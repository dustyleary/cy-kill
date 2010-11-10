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

    static std::string GtpSuccess(const std::string& msg);
    static std::string GtpFailure(const std::string& msg);

    std::string run_cmd(const std::string& in);

    typedef std::string (Gtp::*GtpCommandMethod)(const GtpCommand& in);

    typedef std::map<std::string, GtpCommandMethod> MethodMap;
    MethodMap command_methods;

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
};


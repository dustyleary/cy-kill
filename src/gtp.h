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

    void run(int argc, char** argv);

//private:
    static std::string preprocess_line(std::string line);
    static bool parse_line(const std::string& line, GtpCommand& result);

    static std::string GtpSuccess();
    static std::string GtpSuccess(const std::string& msg);
    static std::string GtpFailure(const std::string& msg, const GtpCommand& gc);

    std::string run_cmd(const std::string& in);

    typedef std::string (Gtp::*GtpCommandMethod)(const GtpCommand& in);

    typedef std::map<std::string, GtpCommandMethod> MethodMap;
    MethodMap m_commandMethods;

    template<typename T>
    void registerMethod(const std::string& cmd_name, std::string (T::*gcm)(const GtpCommand&)) {
        m_commandMethods[cmd_name] = (MethodMap::mapped_type)gcm;
    }

    typedef std::map<std::string, uint*> IntParamMap;
    IntParamMap m_intParams;
    void registerIntParam(uint* v, const std::string& label);

    typedef std::map<std::string, double*> DoubleParamMap;
    DoubleParamMap m_doubleParams;
    void registerDoubleParam(double* v, const std::string& label);

    std::string buffer_io(const GtpCommand& gc);
    std::string echo_text(const GtpCommand& gc);
    std::string engine_param(const GtpCommand& gc);
    std::string gogui_analyze_commands(const GtpCommand& gc);
    std::string gogui_interrupt(const GtpCommand& gc);
    std::string known_command(const GtpCommand& gc);
    std::string list_commands(const GtpCommand& gc);
    std::string name(const GtpCommand& gc);
    std::string protocol_version(const GtpCommand& gc);
    std::string quit(const GtpCommand& gc);
    std::string version(const GtpCommand& gc);

    void input_thread();

    volatile bool needs_interrupt();
    void clear_interrupt();

    volatile bool _needs_interrupt;

    FILE* fin;
    FILE* fout;
    FILE* ferr;
    std::list<std::string> input_lines;
    Mutex input_mutex;
};

std::string trim(const std::string& in);
bool is_integer(const std::string& token);
bool is_double(const std::string& token);
int parse_integer(const std::string& token);
double parse_double(const std::string& token);


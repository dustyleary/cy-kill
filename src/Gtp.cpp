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

void Gtp::registerIntParam(uint* v, const std::string& label) {
    m_intParams[label] = v;
}

void Gtp::registerDoubleParam(double* v, const std::string& label) {
    m_doubleParams[label] = v;
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

std::string Gtp::echo_text(const GtpCommand& gc) {
    std::string result = "";
    for(uint i=0; i<gc.args.size(); i++) {
        if(!result.empty()) result += " ";
        result += gc.args[i];
    }
    return GtpSuccess(result);
}

std::string Gtp::gogui_analyze_commands(const GtpCommand& gc) {
    return GtpSuccess("param/Engine Params/engine_param");
}

std::string Gtp::gogui_interrupt(const GtpCommand& gc) {
    return GtpSuccess();
}

volatile bool Gtp::needs_interrupt() {
    return _needs_interrupt;
}

void Gtp::clear_interrupt() {
    _needs_interrupt = false;
}

Gtp::Gtp(FILE* fin, FILE* fout, FILE* ferr)
    : fin(fin), fout(fout), ferr(ferr)
    , _needs_interrupt(false)
{
    if(fout) setbuf(fout, NULL);
    if(ferr) setbuf(ferr, NULL);

    registerMethod("buffer_io", &Gtp::buffer_io);
    registerMethod("echo_text", &Gtp::echo_text);

    registerMethod("engine_param", &Gtp::engine_param);
    registerMethod("gogui-analyze_commands", &Gtp::gogui_analyze_commands);
    registerMethod("gogui-interrupt", &Gtp::gogui_interrupt);
    registerMethod("known_command", &Gtp::known_command);
    registerMethod("list_commands", &Gtp::list_commands);
    registerMethod("name", &Gtp::name);
    registerMethod("protocol_version", &Gtp::protocol_version);
    registerMethod("quit", &Gtp::quit);
    registerMethod("version", &Gtp::version);
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

std::string Gtp::run_cmd(const std::string& in) {
    GtpCommand gc;
    if(!parse_line(in, gc)) {
        return GtpFailure("parse error", gc);
    }
    std::string echo_back = gc.command;
    for(uint i=0; i<gc.args.size(); i++) {
      echo_back += ' ';
      echo_back += gc.args[i];
    }
    fprintf(stderr, "# cmd '%s'\n", echo_back.c_str());

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

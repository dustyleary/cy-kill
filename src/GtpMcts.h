#pragma once

template<typename GAME>
class GtpMcts : public Gtp {
public:
    typedef typename GAME::Move Move;
    typedef typename OpeningBook<GAME>::BookMoveInfo BookMoveInfo;

    GtpMcts(FILE* fin=stdin, FILE* fout=stdout, FILE* ferr=stderr)
        : Gtp(fin,fout,ferr)
    {
        registerMethod("clear_board", &GtpMcts<GAME>::clear_board);
        registerMethod("dump_board", &GtpMcts<GAME>::dump_board);
        registerMethod("genmove", &GtpMcts<GAME>::genmove);
        registerMethod("play", &GtpMcts<GAME>::play);
        registerMethod("gogui-play_sequence", &GtpMcts<GAME>::gogui_play_sequence);
        registerMethod("showInterestingMoves", &GtpMcts<GAME>::showInterestingMoves);

        registerAnalyzeCommand("gfx/Show Opening Book Interesting Moves/showInterestingMoves");

        m_random_seed = 0;
        max_think_millis = 1000 * 60 * 60;
        max_traces = 100000000;

        book_interesting_move_traces_fraction = 0.2;

        registerIntParam(&m_random_seed, "random_seed");
        registerIntParam(&max_traces, "max_traces");
        registerIntParam(&max_think_millis, "max_think_millis");

        book_movelocal_interesting_fraction = 0.1;
        book_boardlocal_interesting_fraction = 0.1;

        registerDoubleParam(&book_movelocal_interesting_fraction, "book_movelocal_interesting_fraction");
        registerDoubleParam(&book_boardlocal_interesting_fraction, "book_boardlocal_interesting_fraction");
        registerDoubleParam(&book_interesting_move_traces_fraction, "book_interesting_move_traces_fraction");

        uct_kRaveEquivalentPlayouts = 100;
        uct_kUctC = sqrt(2.0);
        uct_kCountdownToCertainty = 100000;
        uct_kGuiShowMoves = 10;
        uct_kMinVisitsForCertainty = 5000;
        uct_kModuloPlayoutsDenominator = 1;
        uct_kModuloPlayoutsNumerator = 0;
        uct_kNumPlayoutsPerTrace = 1;
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

    static bool parseGtpColor(const std::string& in, PointColor& out) {
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

    static bool parseGtpVertex(const std::string& in, Point& out) {
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
        int y = parse_integer(num);
        if(y<1) return false;
        y -= 1;
        out = COORD(x, y);
        return true;
    }

    std::string clear_board(const GtpCommand& gc) {
        uint seed = m_random_seed;
        if(seed == 0) {
            seed = cykill_millisTime();
        }
        LOG("using random seed: %d", seed);
        init_gen_rand(seed);
        m_board.reset();
        dump_board(gc);
        return GtpSuccess();
    }

    std::string dump_board(const GtpCommand& gc) {
        m_board.dump();
        fprintf(stderr, "#w: %c\n", m_board.winner().stateChar());
        return GtpSuccess();
    }

    std::vector<Move> getInterestingMoves() {
        PointColor color = m_board.getWhosTurn();

        std::vector<Move> validMoves;
        m_board.getValidMoves(color, validMoves);
        int validMoveCount = validMoves.size();

        if(mOpeningBook) {
            std::vector<Move> result;

            std::vector<BookMoveInfo> interestingMoves;

            interestingMoves = mOpeningBook->getInterestingMoves_movelocal(m_board, color);
            while(interestingMoves.size() > validMoveCount * book_movelocal_interesting_fraction) {
                interestingMoves.pop_back();
            }
            LOG("Book.getInterestingMoves_movelocal(): %d", interestingMoves.size());
            for(uint im=0; im<interestingMoves.size(); im++) {
                BookMoveInfo& bmi = interestingMoves[im];
                LOG("    %s", bmi.toString().c_str());
                result.push_back(bmi.move);
            }

            interestingMoves = mOpeningBook->getInterestingMoves_boardlocal(m_board, color);
            while(interestingMoves.size() > validMoveCount * book_boardlocal_interesting_fraction) {
                interestingMoves.pop_back();
            }
            LOG("Book.getInterestingMoves_boardlocal(): %d", interestingMoves.size());
            for(uint im=0; im<interestingMoves.size(); im++) {
                BookMoveInfo& bmi = interestingMoves[im];
                LOG("    %s", bmi.toString().c_str());
                result.push_back(bmi.move);
            }

            return result;
        } else {
            fprintf(stderr, "# (no opening book)");
            return std::vector<Move>();
        }
    }

    std::string showInterestingMovesText() {

        std::vector<Move> interestingMoves = getInterestingMoves();

        std::string gfxText = "COLOR green";
        for(uint im=0; im<interestingMoves.size(); im++) {
            Move m = interestingMoves[im];
            gfxText += ' ';
            gfxText += m.toString().c_str();
        }
        gfxText += '\n';

        return gfxText;
    }

    std::string showInterestingMoves(const GtpCommand& gc) {
        return GtpSuccess(showInterestingMovesText());
    }

    std::string genmove(const GtpCommand& gc) {
        if(gc.args.size() != 1) {
            return GtpFailure("syntax error", gc);
        }
        PointColor color;
        if(!parseGtpColor(gc.args[0], color)) {
            return GtpFailure("syntax error", gc);
        }

        fprintf(stderr, "gogui-gfx: CLEAR\n");

        Move bestMove;

        if(mOpeningBook) {
            std::vector<BookMoveInfo> bookMoveInfos = mOpeningBook->getBookMoves(m_board, color);
            LOG("Book.getBookMoves(): %d", bookMoveInfos.size());
            for(uint im=0; im<bookMoveInfos.size(); im++) {
                LOG("    %s", bookMoveInfos[im].toString().c_str());
            }
            if(!bookMoveInfos.empty()) {
                //bestMove = moves[gen_rand64() % moves.size()];
            }
        }

        if(bestMove.color == PointColor::EMPTY()) {
            Mcts2<GAME> mcts;

            mcts.gfxPost = showInterestingMovesText();

            std::vector<Move> interestingMoves = getInterestingMoves();

#define MCTS_FIELD(f) \
            mcts.f = uct_ ## f; \
            LOG("mcts." #f ": %f", (float)mcts.f);

            MCTS_FIELD(kTracesPerGuiUpdate);
            MCTS_FIELD(kGuiShowMoves);
            MCTS_FIELD(kUctC);
            MCTS_FIELD(kRaveEquivalentPlayouts);
            MCTS_FIELD(kMinVisitsForCertainty);
            MCTS_FIELD(kCountdownToCertainty);
            MCTS_FIELD(kNumPlayoutsPerTrace);
            MCTS_FIELD(kModuloPlayoutsNumerator);
            MCTS_FIELD(kModuloPlayoutsDenominator);

            uint32_t st = cykill_millisTime();
            uint32_t et;
            while(true) {
                et = cykill_millisTime();
                if((et-st) > max_think_millis) {
                    break;
                }
                if(mcts.total_traces > max_traces) {
                    break;
                }
                if(mcts.gotMoveCertainty >= 1) {
                    break;
                }
                if(needs_interrupt()) {
                    clear_interrupt();
                    break;
                }
                std::vector<Move>* restrictFirstMoves = 0;
                if(!interestingMoves.empty() && mcts.total_traces < book_interesting_move_traces_fraction * max_traces) {
                    restrictFirstMoves = &interestingMoves;
                }
                mcts.step(m_board, color, restrictFirstMoves);
            }

            bestMove = mcts.getBestMove(m_board, color);

            fprintf(stderr, "# total time: %.2f\n\n", (et-st)/1000.0);
            mcts.gogui_info(m_board, color);
        }

        m_board.playMoveAssumeLegal(bestMove);
        dump_board(gc);

        return GtpSuccess(bestMove.toString());
    }

    std::string play(const GtpCommand& gc) {
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

        dump_board(gc);

        return GtpSuccess();
    }

    std::string gogui_play_sequence(const GtpCommand& gc) {
        if(gc.args.size() < 2) {
            return GtpFailure("syntax error", gc);
        }
        if(gc.args.size() % 2 != 0) {
            return GtpFailure("syntax error", gc);
        }
        for(uint i=0; i<gc.args.size(); i+=2) {
            PointColor color;
            if(!parseGtpColor(gc.args[i+0], color)) {
                return GtpFailure("syntax error", gc);
            }
            Point vertex;
            if(!parseGtpVertex(gc.args[i+1], vertex)) {
                return GtpFailure("syntax error", gc);
            }
            Move m(color, vertex);
            if(!m_board.isValidMove(m)) {
                return GtpFailure("illegal move", gc);
            }
            m_board.playMoveAssumeLegal(m);
        }
        return GtpSuccess();
    }

protected:
    boost::shared_ptr<OpeningBook<GAME> > mOpeningBook;
    void setOpeningBook(boost::shared_ptr<OpeningBook<GAME> > p) {
        mOpeningBook = p;
    }

    GAME m_board;

    uint m_random_seed;
    uint max_traces;
    uint max_think_millis;

    double book_movelocal_interesting_fraction;
    double book_boardlocal_interesting_fraction;

    double book_interesting_move_traces_fraction;

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



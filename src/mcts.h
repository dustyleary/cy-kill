#pragma once

struct Mcts {
    const uint kPlayouts;
    const uint kExpandThreshold;
    const uint kStepSize;
    const double kUctK;
    const double kRaveEquivalentWins;

    struct Node {
        uint64_t zobrist;
        uint num_visits;
        uint num_wins;
        uint num_rave_visits;
        uint num_rave_wins;
        Node(uint64_t z=0, Node* parent=0)
            : num_wins(0)
            , num_visits(0)
            , num_rave_wins(0)
            , num_rave_visits(0)
            , zobrist(z)
            , parent(parent)
        {}
        Node* parent;
        std::map<uint64_t, Node*> children;
    };

    const Board& board;
    BoardState player;
    float komi;
    Point curBestMove;
    uint total_playouts;
    uint32_t startMillis;

    PlayoutResults playoutResults;

    Node root;
    std::map<uint64_t, Node> all_children;
    NatMap<Point, Node*> root_children;

    Mcts(const Board& b,
        float komi,
        BoardState c,
        uint kPlayouts,
        uint kExpandThreshold,
        uint kStepSize,
        double kUctK,
        double kRaveEquivalentWins
        )
        : board(b)
        , komi(komi)
        , player(c)
        , root(0, 0)
        , kPlayouts(kPlayouts)
        , kExpandThreshold(kExpandThreshold)
        , kStepSize(kStepSize)
        , kUctK(kUctK)
        , kRaveEquivalentWins(kRaveEquivalentWins)
        , curBestMove(Point::pass())
        , total_playouts(0)
        , startMillis(cykill_millisTime())
    {
    }

    Node* get_or_make_node(const Board& b, Node* parent) {
        uint64_t z = b.zobrist();
        std::map<uint64_t, Node>::iterator i = all_children.find(z);
        if(i != all_children.end()) {
            return &i->second;
        }
        Node n(z, parent);
        all_children[z] = n;
        parent->children[z] = &all_children[z];
        return &all_children[z];
    }

    void expand_node(const Board& b, BoardState c, Node* n) {
        PointSet moves;
        b.mcgMoves(c, moves);
        for(uint i=0; i<moves.size(); i++) {
            Point p = moves[i];
            Board subboard(b);
            subboard.playMoveAssumeLegal(c, p);
            Node* child = get_or_make_node(subboard, n);
            n->children[child->zobrist] = child;
            terminal_node_playout(subboard, c.enemy(), child);
        }
    }

    void record_node_visit(Node* n, int num_visits, int num_wins) {
        while(n) {
            n->num_visits += num_visits;
            n->num_wins += num_wins;
            n = n->parent;
        }
    }

    void terminal_node_playout(const Board& b, BoardState c, Node* n) {
        PureRandomPlayer rp;
        rp.doPlayouts(
            b,
            kPlayouts,
            komi,
            c,
            playoutResults
        );
        total_playouts += kPlayouts;
    }

    void initRoots() {
        expand_node(board, player, &root);

        PointSet moves;
        board.mcgMoves(player, moves);
        for(uint i=0; i<moves.size(); i++) {
            Point p = moves[i];
            Board subboard(board);
            subboard.playMoveAssumeLegal(player, p);
            Node* child = get_or_make_node(subboard, &root);
            root_children[p] = child;
        }
    }

    double getWinRate(Node* n) {
        if(!n->num_visits) return 1e9;
        return double(n->num_wins) / double(n->num_visits);
    }
    double getUctNumber(Node* n) {
        if(!n->num_visits) return 1e9;
        return kUctK * sqrt(log((double)n->parent->num_visits) / (1 * n->num_visits));
    }
    double getRaveBeta(Node* n) {
    }

    double getMoveWeight(Node* n, BoardState playerColor) {
        double winrate = getWinRate(n);
        if(playerColor != BoardState::BLACK()) {
            winrate = 1 - winrate;
        }
        double uctNumber = getUctNumber(n);
        return winrate + uctNumber;
    }

    void doTrace(Point p, const Board& b, BoardState c, Node* n) {
        if(n->children.empty()) {
            //i am terminal...  perhaps expand
            if(n->num_visits >= kExpandThreshold) {
                expand_node(b, c, n);
                //after expanding, re-invoke myself and I'll continue with one child
                doTrace(p, b, c, n);
            } else {
                //no expand, just do a line
                terminal_node_playout(board, c, n);
                n->num_visits += playoutResults.getPlayouts();
                n->num_wins += playoutResults.black_wins;
            }
        } else {
            //find the best child
            Node* bestChild = NULL;
            double bestValue = -100000;
            Point bestPoint;

            PointSet moves;
            b.mcgMoves(c, moves);
            for(uint i=0; i<moves.size(); i++) {
                Point p = moves[i];
                Board subboard(b);
                subboard.playMoveAssumeLegal(c, p);
                Node* child = get_or_make_node(subboard, n);
                double thisValue = getMoveWeight(child, c);
                if(thisValue >= bestValue) {
                    bestChild = child;
                    bestValue = thisValue;
                    bestPoint = p;
                }
            }
            if(bestChild) {
                //and recurse
                Board subboard(b);
                subboard.playMoveAssumeLegal(c, bestPoint);
                doTrace(bestPoint, subboard, c.enemy(), bestChild);
                n->num_visits += playoutResults.getPlayouts();
                n->num_wins += playoutResults.black_wins;
                if(c == player) {
                    Node* amaf_node = root_children[bestPoint];
                    amaf_node->num_rave_visits += playoutResults.getPlayouts();
                    amaf_node->num_rave_wins += playoutResults.black_wins;
                }
            }
        }
    }

    void doTrace() {
        playoutResults = PlayoutResults();
        doTrace(Point::invalid(), board, player, &root);
    }

    void step() {
        LOG("# step");
        for(uint i=0; i<kStepSize; i++) {
            doTrace();
        }
        gogui_info();
    }

    void gogui_info() {
        PointSet moves;
        board.mcgMoves(player, moves);
        Point bestMove = Point::pass();
        double bestRate = -10;
        uint bestCount = 0;
        std::string influence = "INFLUENCE";
        std::string label = "LABEL";
        uint32_t dt = cykill_millisTime() - startMillis;
        double seconds = double(dt) / 1000.0;
        double kpps = total_playouts / (seconds * 1000);
        std::string text = strprintf("TEXT %d playouts %.1f kpps\n", total_playouts, kpps);
        for(uint i=0; i<moves.size(); i++) {
            Point p = moves[i];
            Board subboard(board);
            subboard.playMoveAssumeLegal(player, p);
            Node* child = get_or_make_node(subboard, &root);
            uint count = child->num_visits;
            double winrate = getWinRate(child);
            if(winrate > bestRate) {
                bestMove = p;
                bestCount = count;
                bestRate = winrate;
            }
            label += strprintf(" %s %d",
                    p.toGtpVertex(board.getSize()).c_str(),
                    count);
            influence += strprintf(" %s %.3f",
                    p.toGtpVertex(board.getSize()).c_str(),
                    winrate);
        }
        std::string cname = (player==BoardState::BLACK()) ? "B" : "W";
        std::string var = strprintf("VAR %s %s", cname.c_str(), bestMove.toGtpVertex(board.getSize()).c_str());
        std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
        fputs(gfx.c_str(), stderr);
        curBestMove = bestMove;
    }
};

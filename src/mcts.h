#pragma once

struct Mcts {
    const uint kPlayouts; //11
    const uint kExpandThreshold; //5
    const uint kStepSize; //100
    const double kUctK; //1.0

    struct Node {
        uint64_t zobrist;
        uint num_visits;
        uint num_wins;
        Node(uint64_t z=0, Node* parent=0)
            : num_wins(0)
            , num_visits(0)
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

    Node root;
    std::map<uint64_t, Node> all_children;

    Mcts(const Board& b,
        float komi,
        BoardState c,
        uint kPlayouts,
        uint kExpandThreshold,
        uint kStepSize,
        double kUctK
        )
        : board(b)
        , komi(komi)
        , player(c)
        , root(0, 0)
        , kPlayouts(kPlayouts)
        , kExpandThreshold(kExpandThreshold)
        , kStepSize(kStepSize)
        , kUctK(kUctK)
        , curBestMove(Point::pass())
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
            n->num_visits+= num_visits;
            n->num_wins += num_wins;
            n = n->parent;
        }
    }

    void terminal_node_playout(const Board& b, BoardState c, Node* n) {
        PlayoutResults r;
        PureRandomPlayer rp;
        rp.doPlayouts(
            b,
            kPlayouts,
            komi,
            c,
            r
        );
        int num_wins = (player == BoardState::WHITE()) ? r.white_wins : r.black_wins;
        record_node_visit(n, kPlayouts, num_wins);
    }

    void initRoots() {
        expand_node(board, player, &root);
    }

    double uctValue(Node* n) {
        double winrate = double(n->num_wins) / double(n->num_visits);
        double uct = winrate + kUctK * sqrt(log((double)n->parent->num_visits) / (5 * n->num_visits));
        return uct;
    }

    void doTrace(const Board& b, BoardState c, Node* n) {
        if(n->children.empty()) {
            //i am terminal...  perhaps expand
            if(n->num_visits >= kExpandThreshold) {
                expand_node(b, c, n);
            } else {
                //no expand, just do a line
                terminal_node_playout(board, c, n);
            }
        } else {
            //find the best child
            Node* bestChild = NULL;
            double bestValue = -100;
            Point bestPoint;

            PointSet moves;
            b.mcgMoves(c, moves);
            for(uint i=0; i<moves.size(); i++) {
                Point p = moves[i];
                Board subboard(b);
                subboard.playMoveAssumeLegal(c, p);
                Node* child = get_or_make_node(subboard, n);
                if(bestChild == NULL) {
                    bestChild = child;
                    bestValue = uctValue(child);
                    bestPoint = p;
                }
                double thisValue = uctValue(child);
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
                doTrace(subboard, c.enemy(), bestChild);
            }
        }
    }

    void doTrace() {
        doTrace(board, player, &root);
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
        for(uint i=0; i<moves.size(); i++) {
            Point p = moves[i];
            Board subboard(board);
            subboard.playMoveAssumeLegal(player, p);
            Node* child = get_or_make_node(subboard, &root);
            uint count = child->num_visits;
            double winrate = double(child->num_wins) / double(child->num_visits);
            if(count > bestCount) {
                bestMove = p;
                bestCount = count;
                bestRate = winrate;
            } else if(count == bestCount) {
                if(winrate > bestRate) {
                    bestMove = p;
                    bestCount = count;
                    bestRate = winrate;
                }
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
        std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n\n";
        fputs(gfx.c_str(), stderr);
        curBestMove = bestMove;
    }
};

#pragma once

struct Mcts {
    const uint kPlayouts;
    const uint kExpandThreshold;
    const uint kTracesPerGuiUpdate;
    const double kUctK;
    const double kUctRaveEquivalentSimulationsCount; //aka 'k' in paper

    struct Node {
        uint64_t zobrist;
        uint num_real_visits;
        uint num_real_wins;
        uint num_virtual_visits;
        uint num_virtual_wins;
        Node* parent;
        std::map<uint64_t, Node*> children;

        Node(uint64_t z=0, Node* parent=0)
            : num_real_wins(0)
            , num_real_visits(0)
            , num_virtual_wins(0)
            , num_virtual_visits(0)
            , zobrist(z)
            , parent(parent)
        {}
        Node(const Node& rhs) {
            memcpy(this, &rhs, sizeof(rhs));
        }
    };

    const Board& board;
    BoardState player;
    float komi;
    Point curBestMove;
    uint total_playouts;

    Node root;
    std::map<uint64_t, Node> all_children;
    NatMap<Point, Node*> root_amaf_children;

    Mcts(const Board& b,
        float komi,
        BoardState c,
        uint kPlayouts,
        uint kExpandThreshold,
        uint kTracesPerGuiUpdate,
        double kUctK,
        double kUctRaveEquivalentSimulationsCount
        )
        : board(b)
        , komi(komi)
        , player(c)
        , root(0, 0)
        , kPlayouts(kPlayouts)
        , kExpandThreshold(kExpandThreshold)
        , kTracesPerGuiUpdate(kTracesPerGuiUpdate)
        , kUctK(kUctK)
        , kUctRaveEquivalentSimulationsCount(kUctRaveEquivalentSimulationsCount)
        , curBestMove(Point::pass())
        , total_playouts(0)
    {
        root.num_real_visits = 1;
    }

    Node* get_or_make_node(const Board& b, Node* parent, Point pt, BoardState color) {
        uint64_t z = b.zobrist();
        std::map<uint64_t, Node>::iterator i = all_children.find(z);
        if(i != all_children.end()) {
            Node* result = &i->second;
            ASSERT(result->zobrist == z);
            return result;
        }
        Node n(z, parent);
        all_children[z] = n;
        //LOG("sz: %d", all_children.size());
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
            Node* child = get_or_make_node(subboard, n, p, c);
            n->children[child->zobrist] = child;
            if(n == &root) {
                root_amaf_children[p] = child;
            }
        }
    }

    void record_node_visit(Node* n, int visits, int wins, BoardState c, Point first_move) {
        while(n) {
            n->num_real_visits += visits;
            n->num_real_wins += wins;
            Node* amaf_sibling = root_amaf_children[first_move];
            if(amaf_sibling) {
                amaf_sibling->num_virtual_visits += visits;
                amaf_sibling->num_virtual_wins += wins;
            }
            n = n->parent;
        }
    }

    void terminal_node_playout(const Board& b, BoardState c, Node* n, Point first_move) {
        PlayoutResults r;
        PureRandomPlayer rp;
        rp.doPlayouts(
            b,
            kPlayouts,
            komi,
            c,
            r
        );
        total_playouts += kPlayouts;
        record_node_visit(n, kPlayouts, r.black_wins, c, first_move);
    }

    void initRoots() {
        expand_node(board, player, &root);
    }

    double winrate(Node* n) {
        if(!n->num_real_visits) {
            return 1e9;
        }
        return double(n->num_real_wins) / double(n->num_real_visits);
    }

    double virtual_winrate(Node* n) {
        if(!n->num_real_visits) {
            return 1e9;
        }
        return double(n->num_real_wins + n->num_virtual_wins) / double(n->num_real_visits + n->num_virtual_visits);
    }

    double beta(Node* n) {
        double k = kUctRaveEquivalentSimulationsCount;
        return sqrt(k / (3*n->num_real_visits + k));
    }

    double uctValue(Node* n) {
        return kUctK * sqrt(log((double)n->parent->num_real_visits) / (1 + n->num_real_visits));
    }

    double nodeChoiceStrength(Node* n) {
        double b = beta(n);
        return b*winrate(n) + (1-b)*virtual_winrate(n) + uctValue(n);
    }

    void doTrace(const Board& b, BoardState c, Node* n, Point first_move) {
        if(n->children.empty()) {
            //i am terminal...  perhaps expand
            if(n->num_real_visits >= kExpandThreshold) {
                expand_node(b, c, n);
            } else {
                //no expand, just do a line
                terminal_node_playout(board, c, n, first_move);
            }
        } else {
            //find the best child
            Node* bestChild = NULL;
            double bestValue = -1e9;
            Point bestPoint;
            
            PointSet moves;
            b.mcgMoves(c, moves);
            for(uint i=0; i<moves.size(); i++) {
                Point p = moves[i];
                Board subboard(b);
                subboard.playMoveAssumeLegal(c, p);
                Node* child = get_or_make_node(subboard, n, p, c);
                double thisValue = nodeChoiceStrength(child);
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
                if(!first_move.isValid()) {
                    first_move = bestPoint;
                }
                doTrace(subboard, c.enemy(), bestChild, first_move);
            }
        }
    }

    void doTrace() {
        doTrace(board, player, &root, Point::invalid());
    }

    void step() {
        LOG("# step");
        for(uint i=0; i<kTracesPerGuiUpdate; i++) {
            doTrace();
        }
        gogui_info();
    }

    void gogui_info() {
        PointSet moves;
        board.mcgMoves(player, moves);
        Point bestMove = Point::pass();
        uint bestCount = 0;
        double bestRate = 0;
        std::string influence = "INFLUENCE";
        std::string label = "LABEL";
        std::string text = strprintf("TEXT %d playouts\n", total_playouts);
        for(uint i=0; i<moves.size(); i++) {
            Point p = moves[i];
            Board subboard(board);
            subboard.playMoveAssumeLegal(player, p);
            Node* child = get_or_make_node(subboard, &root, p, player);
            uint count = child->num_real_visits;
            double rate = child->num_real_wins / ((double)child->num_real_visits+0.01);
            //if(count > bestCount) {
            //    bestMove = p;
            //    bestCount = count;
            //}
            if(rate > bestRate) {
                bestMove = p;
                bestRate = rate;
            }
            label += strprintf(" %s %d",
                    p.toGtpVertex(board.getSize()).c_str(),
                    count);
            text += strprintf(" %s %.3f-%.3f-%.3f",
                    p.toGtpVertex(board.getSize()).c_str(),
                    winrate(child), virtual_winrate(child), uctValue(child));
            influence += strprintf(" %s %.3f",
                    p.toGtpVertex(board.getSize()).c_str(),
                    winrate(child));
        }
        std::string cname = (player==BoardState::BLACK()) ? "B" : "W";
        std::string var = strprintf("VAR %s %s", cname.c_str(), bestMove.toGtpVertex(board.getSize()).c_str());
        std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
        fputs(gfx.c_str(), stderr);
        curBestMove = bestMove;
    }
};

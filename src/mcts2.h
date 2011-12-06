#pragma once

namespace {

using boost::shared_ptr;
using namespace boost::tuples;

template<class BOARD>
struct Mcts2 {

  double kUctC;
  double kRaveEquivalentPlayouts;
  double kTracesPerGuiUpdate;

  struct WinStats {
    double num_visits;
    double black_wins;
    WinStats() : num_visits(0), black_wins(0) {}
  };

  struct Node {
    uint64_t zobrist;
    WinStats winStats;

    typedef std::map<typename BOARD::Move, Node*> ChildMap;

    ChildMap children;

    Node(uint64_t zobrist) : zobrist(zobrist) {
    }

    bool isLeafNode() const {
      return winStats.num_visits == 0;
    }
  };

  typedef std::map<uint64_t, Node> AllNodesMap;
  AllNodesMap all_nodes;

  typedef std::map<typename BOARD::Move, WinStats> AmafWinStats;
  AmafWinStats amafWinStats;

  typedef shared_ptr<WeightedRandomChooser> ChooserPtr;
  ChooserPtr mChooser;
  uint total_playouts;
  uint startTime;

  Mcts2(WeightedRandomChooser* wrc = 0) {
    total_playouts = 0;
    mChooser = ChooserPtr(wrc);
    if(!mChooser) {
      mChooser = ChooserPtr(new WeightedRandomChooser());
    }
    startTime = cykill_millisTime();
  }

  Node* getNodeForBoard(const BOARD& b) {
    uint64_t zobrist = b.zobrist();
    typename AllNodesMap::iterator i1 = all_nodes.find(zobrist);
    if(i1 == all_nodes.end()) {
      i1 = all_nodes.insert(typename AllNodesMap::value_type(zobrist, Node(zobrist))).first;
    }
    return &i1->second;
  }

  double getValueEstimate(BoardState playerColor, WinStats& winStats) {
    ASSERT(winStats.num_visits > 0);

    double wins = (playerColor == BoardState::BLACK()) ? winStats.black_wins : (winStats.num_visits - winStats.black_wins);
    double value = wins / winStats.num_visits;
    return value;
  }

  double getValueEstimate(BoardState playerColor, Node* childNode, Move move) {
    WinStats childStats = childNode->winStats;
    WinStats amafStats = amafWinStats[move];

    double amaf_alpha = 1.0 - (childStats.num_visits / (2*kRaveEquivalentPlayouts));
    if(amaf_alpha<0) amaf_alpha = 0;
    WinStats useStats;
    useStats.num_visits = (1.0-amaf_alpha)*childStats.num_visits + amaf_alpha*amafStats.num_visits;
    useStats.black_wins = (1.0-amaf_alpha)*childStats.black_wins + amaf_alpha*amafStats.black_wins;

    return getValueEstimate(playerColor, useStats);
  }

  double getUctWeight(BoardState playerColor, double logParentVisitCount, Node* childNode, Move move) {
    double vi = getValueEstimate(playerColor, childNode, move);
    if(childNode->winStats.num_visits == 0) {
      return vi;
    }
    return vi + kUctC * sqrt(logParentVisitCount / childNode->winStats.num_visits);
  }

  typedef tuple<BOARD, std::list<Node*>, std::list<typename BOARD::Move> > TreewalkResult;

  TreewalkResult doUctTreewalk(const BOARD& b, BoardState playerColor) {
    BOARD subboard(b);
    std::list<Node*> visited_nodes;
    std::list<typename BOARD::Move> visited_moves;

    Node* node = getNodeForBoard(subboard);
    bool two_passes = false;

    while(true) {
      visited_nodes.push_back(node);

      if(two_passes || node->isLeafNode()) {
        return TreewalkResult(subboard, visited_nodes, visited_moves);
      }

      double logParentVisitCount = log(node->winStats.num_visits);

      //build the weighted choice for our child nodes
      std::vector<typename BOARD::Move> moves;
      subboard.getValidMoves(playerColor, moves);

      //subboard.dump();

      std::vector<double> weights(moves.size());
      double weights_sum = 0;
      for(uint i=0; i<moves.size(); i++) {
        double weight = 1e5; //weight of not-found nodes
        typename Node::ChildMap::iterator ci = node->children.find(moves[i]);
        Node* childNode = 0;
        if(ci != node->children.end()) {
          childNode = ci->second;
          weight = getUctWeight(playerColor, logParentVisitCount, childNode, moves[i]);
        }
        //LOG("%s 0x%08x %.2f", moves[i].point.toGtpVertex(subboard.getSize()).c_str(), childNode, weight);
        weights[i] = weight;
        weights_sum += weight;
      }

      //choose a move
      int idx = mChooser->choose((uint)weights.size(), &weights[0], weights_sum);
      Move move;
      if(idx == -1) {
        move = Move(playerColor, Point::pass());
      } else {
        move = moves[idx];
      }
      //LOG("%14.2f %d", weights_sum, idx);

      if(visited_moves.begin() != visited_moves.end()) {
        if(move.point == Point::pass()) {
          if(visited_moves.back().point == Point::pass()) {
            two_passes = true;
          }
        }
      }
      
      
      //make the move
      subboard.playMoveAssumeLegal(move);
      Node* childNode = getNodeForBoard(subboard);
      node->children[move] = childNode;
      node = childNode;
      playerColor = playerColor.enemy();
      visited_moves.push_back(move);


    }
  }

  void doTrace(const BOARD& _b, BoardState _playerColor, RandomPlayerPtr randomPlayer) {
    //walk tree
    TreewalkResult twr = doUctTreewalk(_b, _playerColor);

    const Board& playoutBoard = get<0>(twr);
    const std::list<Node*> visitedNodes = get<1>(twr);
    const std::list<typename BOARD::Move> visitedMoves = get<2>(twr);
    BoardState playoutColor = playoutBoard.getWhosTurn();

    //do playout
    PlayoutResults pr;
    randomPlayer->doPlayouts(playoutBoard, 1, playoutColor, pr);
    ++total_playouts;

    //update visited nodes
    typename std::list<Node*>::const_reverse_iterator i1 = visitedNodes.rbegin();
    while(i1 != visitedNodes.rend()) {
      Node* node = *i1;
      ++i1;
      WinStats& winStats = node->winStats;

      winStats.num_visits += (pr.black_wins + pr.white_wins);
      winStats.black_wins += pr.black_wins;
    }

    //update visited moves
    typename std::list<typename BOARD::Move>::const_iterator i2 = visitedMoves.begin();
    while(i2 != visitedMoves.end()) {
      Move m = *i2;
      ++i2;
      WinStats& winStats = amafWinStats[m];

      winStats.num_visits += (pr.black_wins + pr.white_wins);
      winStats.black_wins += pr.black_wins;
    }
  }

  void step(const BOARD& _b, BoardState _playerColor, RandomPlayerPtr randomPlayer) {
    LOG("# step");
    for(uint i=0; i<kTracesPerGuiUpdate; i++) {
        doTrace(_b, _playerColor, randomPlayer);
    }
    gogui_info();
  }

  void gogui_info() {
    uint ct = cykill_millisTime();
    uint millis = ct - startTime;

    std::string text = strprintf("TEXT %d playouts %d/s\n", total_playouts, total_playouts * 1000 / (millis+1));

    //std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
    std::string gfx = "gogui-gfx:\n"+text+"\n\n";
    fputs(gfx.c_str(), stderr);
  }

  Move getBestMove(const BOARD& b, BoardState playerColor) {
    std::vector<typename BOARD::Move> moves;
    b.getValidMoves(playerColor, moves);

    Move bestMove = moves[0];
    double bestValue = -1e6;

    Node* rootNode = getNodeForBoard(b);

    double logParentVisitCount = log(rootNode->winStats.num_visits);

    for(uint i=0; i<moves.size(); i++) {
      typename Node::ChildMap::iterator ci = rootNode->children.find(moves[i]);
      if(ci != rootNode->children.end()) {
        Node* childNode = ci->second;
        double value = getUctWeight(playerColor, logParentVisitCount, childNode, moves[i]);
        LOG("move candidate: %s visits: %d black_wins: %d value: %.2f",
            moves[i].point.toGtpVertex(b.getSize()).c_str(),
            (int)childNode->winStats.num_visits,
            (int)childNode->winStats.black_wins,
            value
           );
        if(value > bestValue) {
          bestMove = moves[i];
          bestValue = value;
        }
      }
    }

    return bestMove;
  }
};

}


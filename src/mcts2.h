#pragma once

namespace {

using boost::shared_ptr;
using namespace boost::tuples;

template<class BOARD>
struct Mcts2 {

  double kUctC;
  double kRaveEquivalentPlayouts;
  uint kTracesPerGuiUpdate;
  uint kGuiShowMoves;
  uint kMinVisitsForCertainty;
  uint kCountdownToCertainty;
  uint kNumPlayoutsPerTrace;

  uint kModuloPlayoutsNumerator;
  uint kModuloPlayoutsDenominator;

  double gotMoveCertainty;
  Move countdownMove;
  int countdown;

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
    gotMoveCertainty = 0;
    countdown = kCountdownToCertainty;
  }

  Node* getNodeForBoard(const BOARD& b) {
    uint64_t zobrist = b.zobrist();
    typename AllNodesMap::iterator i1 = all_nodes.find(zobrist);
    if(i1 == all_nodes.end()) {
      i1 = all_nodes.insert(typename AllNodesMap::value_type(zobrist, Node(zobrist))).first;
    }
    return &i1->second;
  }

  double getValueEstimate(PointColor playerColor, WinStats& winStats) {
    ASSERT(winStats.num_visits > 0);

    double wins = (playerColor == PointColor::BLACK()) ? winStats.black_wins : (winStats.num_visits - winStats.black_wins);
    double value = wins / winStats.num_visits;
    return value;
  }

  double getValueEstimate(PointColor playerColor, Node* childNode, Move move) {
    WinStats childStats = childNode->winStats;
    WinStats amafStats = amafWinStats[move];

    double amaf_alpha = 1.0 - (childStats.num_visits / (2*kRaveEquivalentPlayouts));
    if(amaf_alpha<0) amaf_alpha = 0;
    WinStats useStats;
    useStats.num_visits = (1.0-amaf_alpha)*childStats.num_visits + amaf_alpha*amafStats.num_visits;
    useStats.black_wins = (1.0-amaf_alpha)*childStats.black_wins + amaf_alpha*amafStats.black_wins;

    return getValueEstimate(playerColor, useStats);
  }

  double getUctWeight(PointColor playerColor, double logParentVisitCount, Node* childNode, Move move) {
    double vi = getValueEstimate(playerColor, childNode, move);
    if(childNode->winStats.num_visits == 0) {
      return vi;
    }
    return vi + kUctC * sqrt(logParentVisitCount / childNode->winStats.num_visits);
  }

  typedef tuple<BOARD, std::list<Node*>, std::list<typename BOARD::Move> > TreewalkResult;

  TreewalkResult doUctTreewalk(const BOARD& b, PointColor playerColor) {
    BOARD subboard(b);
    std::list<Node*> visited_nodes;
    std::list<typename BOARD::Move> visited_moves;

    Node* node = getNodeForBoard(subboard);
    bool two_passes = false;

    bool use_modulo = true;

    while(true) {
      visited_nodes.push_back(node);

      if(two_passes || node->isLeafNode()) {
        return TreewalkResult(subboard, visited_nodes, visited_moves);
      }

      double logParentVisitCount = log(node->winStats.num_visits);

      //build the weighted choice for our child nodes
      uint moduloNumerator = 0;
      uint moduloDenominator = 1;
      if(use_modulo) {
        moduloNumerator = kModuloPlayoutsNumerator;
        moduloDenominator = kModuloPlayoutsDenominator;
        use_modulo = false;
      }
      std::vector<typename BOARD::Move> moves;
      subboard.getValidMoves(playerColor, moves, moduloNumerator, moduloDenominator);

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

  void doTrace(const BOARD& _b, PointColor _playerColor, RandomPlayerPtr randomPlayer) {
    //walk tree
    TreewalkResult twr = doUctTreewalk(_b, _playerColor);

    const Board& playoutBoard = get<0>(twr);
    const std::list<Node*> visitedNodes = get<1>(twr);
    const std::list<typename BOARD::Move> visitedMoves = get<2>(twr);
    PointColor playoutColor = playoutBoard.getWhosTurn();

    //do playout
    PlayoutResults pr;
    randomPlayer->doPlayouts(playoutBoard, kNumPlayoutsPerTrace, playoutColor, pr);
    ++total_playouts;
    --countdown;

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

  void step(const BOARD& _b, PointColor _playerColor, RandomPlayerPtr randomPlayer) {
    LOG("# step");
    for(uint i=0; i<kTracesPerGuiUpdate; i++) {
        doTrace(_b, _playerColor, randomPlayer);
    }
    gogui_info(_b, _playerColor);
  }

  int getMaxTreeDepth(const BOARD& b, Board::Move move) {
    if(move.point == Point::pass()) {
      return 0;
    }
    Node* rootNode = getNodeForBoard(b);
    typename Node::ChildMap::iterator ci = rootNode->children.find(move);
    if(ci == rootNode->children.end()) {
      return 0;
    }
    //LOG("getTreeDepth %2s", move.point.toGtpVertex(b.getSize()).c_str());

    BOARD subboard(b);
    subboard.playMoveAssumeLegal(move);

    std::vector<typename BOARD::Move> moves;
    subboard.getValidMoves(move.playerColor.enemy(), moves);

    int max = 0;
    for(uint i=0; i<moves.size(); i++) {
      max = std::max(max, getMaxTreeDepth(subboard, moves[i]));
    }

    return 1 + max;
  }

  static const int BAD_MIN=0;
  int getMinTreeDepth(const BOARD& b, Board::Move move) {
    if(move.point == Point::pass()) {
      return BAD_MIN;
    }
    Node* rootNode = getNodeForBoard(b);
    typename Node::ChildMap::iterator ci = rootNode->children.find(move);
    if(ci == rootNode->children.end()) {
      return BAD_MIN;
    }
    //LOG("getTreeDepth %2s", move.point.toGtpVertex(b.getSize()).c_str());

    BOARD subboard(b);
    subboard.playMoveAssumeLegal(move);

    std::vector<typename BOARD::Move> moves;
    subboard.getValidMoves(move.playerColor.enemy(), moves);

    int min = 0;
    for(uint i=0; i<moves.size(); i++) {
      min = std::min(min, getMinTreeDepth(subboard, moves[i]));
    }

    return 1 + min;
  }

  void gogui_info(const BOARD& b, PointColor playerColor) {
    std::vector<NodeValue> nodeValues;
    rankMoves(b, playerColor, nodeValues);

    uint ct = cykill_millisTime();
    uint millis = ct - startTime;

    std::string text;
    if(kGuiShowMoves>0) {
      text += strprintf("SQUARE %s\n", get<2>(nodeValues[0]).point.toGtpVertex(b.getSize()).c_str());
      int moveCount = std::min(kGuiShowMoves, (uint)nodeValues.size());
      if(moveCount > 1) {
        text += "CIRCLE";
        for(uint i=1; (i<kGuiShowMoves) && (i<nodeValues.size()); i++) {
          text += ' ';
          text += get<2>(nodeValues[i]).point.toGtpVertex(b.getSize());
        }
        text += '\n';
      }
    }

    Move bestMove = get<2>(nodeValues[0]);
    if(bestMove != countdownMove) {
      countdownMove = bestMove;
      countdown = kCountdownToCertainty;
    }

    double min_visits = 1e6;
    for(uint i=0; (i<kGuiShowMoves) && (i<nodeValues.size()); i++) {
      Node* childNode = get<1>(nodeValues[i]);
      if(childNode) {
        min_visits = std::min(min_visits, childNode->winStats.num_visits);
      }
    }

    gotMoveCertainty = 0;
    if(countdown < 0) {
      if(min_visits >= kMinVisitsForCertainty) {
        gotMoveCertainty = 1;
      }
    }

    text += strprintf("TEXT %d playouts %d/s -- countdown: %d -- min_visits: %d\n", total_playouts, total_playouts * 1000 / (millis+1), countdown, (int)min_visits);

    //std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
    std::string gfx = "gogui-gfx:\n"+text+"\n\n";
    fputs(gfx.c_str(), stderr);
  }

  typedef tuple<double, Node*, Move> NodeValue;
  typedef double (Mcts2::*MoveValueFn)(const BOARD& b, PointColor playerColor, Move move, Node* childNode);

  static bool compare(const NodeValue& a, const NodeValue& b) {
    return get<0>(a) > get<0>(b);
  }

  double visits_moveValue(const BOARD& b, PointColor playerColor, Move move, Node* childNode) {
    return childNode->winStats.num_visits;
  }
  double winrate_moveValue(const BOARD& b, PointColor playerColor, Move move, Node* childNode) {
    double black_winrate = childNode->winStats.black_wins / childNode->winStats.num_visits;
    return (playerColor == PointColor::WHITE()) ? (1.0 - black_winrate) : black_winrate;
  }
  double minimizeResponseWinRate_moveValue(const BOARD& b, PointColor playerColor, Move move, Node* childNode) {
    BOARD subboard(b);
    subboard.playMoveAssumeLegal(move);

    std::vector<NodeValue> counterValues;
    rankMoves(subboard, playerColor.enemy(), counterValues, &Mcts2<BOARD>::winrate_moveValue);

    double result = 0;
    if(!counterValues.empty()) {
      result = 1.0 - get<0>(counterValues[0]);
    }
    return result;
  }

  MoveValueFn getMoveValueFn() {
    //return &Mcts2<BOARD>::visits_moveValue;
    //return &Mcts2<BOARD>::winrate_moveValue;
    return &Mcts2<BOARD>::minimizeResponseWinRate_moveValue;
  }

  void rankMoves(const BOARD& b, PointColor playerColor, std::vector<NodeValue>& nodeValues) {
    rankMoves(b, playerColor, nodeValues, getMoveValueFn());
  }

  void rankMoves(const BOARD& b, PointColor playerColor, std::vector<NodeValue>& nodeValues, MoveValueFn moveValueFn) {
    std::vector<typename BOARD::Move> moves;
    b.getValidMoves(playerColor, moves);

    Node* rootNode = getNodeForBoard(b);

    nodeValues.clear();
    nodeValues.reserve(moves.size());

    for(uint i=0; i<moves.size(); i++) {
      typename Node::ChildMap::iterator ci = rootNode->children.find(moves[i]);
      if(ci != rootNode->children.end()) {
        Node* childNode = ci->second;
        double value = (*this.*moveValueFn)(b, playerColor, moves[i], childNode);
        nodeValues.push_back(make_tuple(value, childNode, moves[i]));
      }
    }

    std::sort(nodeValues.begin(), nodeValues.end(), compare);
  }

  static void logNodeValues(const BOARD& b, const std::vector<NodeValue>& nodeValues) {
    for(uint i=0; i<nodeValues.size(); i++) {
      const NodeValue& nv = nodeValues[i];
      LOG("%.2f %08x %2s", get<0>(nv), get<1>(nv), get<2>(nv).point.toGtpVertex(b.getSize()).c_str());
    }
  }

  Move getBestMove(const BOARD& b, PointColor playerColor) {
    std::vector<NodeValue> nodeValues;
    rankMoves(b, playerColor, nodeValues);

    Node* rootNode = getNodeForBoard(b);
    double logParentVisitCount = log(rootNode->winStats.num_visits);

    for(uint i=0; i<nodeValues.size(); i++) {
      double value = get<0>(nodeValues[i]);
      Node* childNode = get<1>(nodeValues[i]);
      Move move = get<2>(nodeValues[i]);
      double uct_weight = getUctWeight(playerColor, logParentVisitCount, childNode, move);

      LOG("move candidate: %2s visits: %6d black_wins: %6d winrate: %.6f value: %.6f uct_weight: %.6f",
          move.point.toGtpVertex(b.getSize()).c_str(),
          (int)childNode->winStats.num_visits,
          (int)childNode->winStats.black_wins,
          float(childNode->winStats.black_wins)/childNode->winStats.num_visits,
          value,
          uct_weight
         );

      BOARD subboard(b);
      subboard.playMoveAssumeLegal(move);
      Node* c_rootNode = getNodeForBoard(subboard);

      double c_logParentVisitCount = log(c_rootNode->winStats.num_visits);

      std::vector<NodeValue> counterValues;
      rankMoves(subboard, playerColor.enemy(), counterValues, &Mcts2<BOARD>::winrate_moveValue);
      for(uint j=0; j<std::min(4, (int)counterValues.size()); j++) {
        double c_value = get<0>(counterValues[j]);
        Node* c_childNode = get<1>(counterValues[j]);
        Move c_move = get<2>(counterValues[j]);
        double c_uct_weight = getUctWeight(playerColor.enemy(), c_logParentVisitCount, c_childNode, c_move);

        LOG("    counter: %2s visits: %6d black_wins: %6d value: %.6f uct_weight: %.6f",
            c_move.point.toGtpVertex(b.getSize()).c_str(),
            (int)c_childNode->winStats.num_visits,
            (int)c_childNode->winStats.black_wins,
            c_value,
            c_uct_weight
           );
      }
    }

    return get<2>(nodeValues[0]);
  }
};

}


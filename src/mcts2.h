#pragma once

#ifdef CYKILL_MT
#define MAP ::tbb::concurrent_unordered_map
namespace tbb {

template<>
class tbb_hash<TwoPlayerGridGame::Move> {
public:
    size_t operator()(const TwoPlayerGridGame::Move& t) const {
        return tbb_hasher(t.color.toUint()) ^ tbb_hasher(t.point.toUint());
    }
};

}
#else
#define MAP std::map
#endif

namespace {

using boost::shared_ptr;
using namespace boost::tuples;

template<class BOARD>
struct Mcts2 {

  typedef typename BOARD::Move Move;

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
    uint games;
    uint black_wins;
    uint white_wins;
    WinStats() : games(0), black_wins(0), white_wins(0) {}
    uint ties() const { return games - black_wins - white_wins; }
    double black_winrate() const { return double(black_wins) / games; }
  };

  struct Node {
    uint64_t zobrist;
    WinStats winStats;

    typedef MAP<Move, Node*> ChildMap;

    ChildMap children;

    Node(uint64_t zobrist) : zobrist(zobrist) {
    }

    bool isLeafNode() const {
      return winStats.games == 0;
    }
  };

  typedef std::map<uint64_t, Node> AllNodesMap;
  AllNodesMap all_nodes;

  typedef std::map<Move, WinStats> AmafWinStats;
  AmafWinStats amafWinStats;

  typedef std::map<Move, size_t> MovePlyDepths;
  MovePlyDepths moveMaxPlies;

  typedef shared_ptr<WeightedRandomChooser> ChooserPtr;
  ChooserPtr mChooser;
  uint total_traces;
  uint startTime;

  Mcts2(WeightedRandomChooser* wrc = 0) {
    total_traces = 0;
    mChooser = ChooserPtr(wrc);
    if(!mChooser) {
      //mChooser = ChooserPtr(new WeightedRandomChooser());
      mChooser = ChooserPtr(new EpsilonGreedyChooser(0.2));
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

  double getValueEstimate(PointColor color, WinStats& winStats) {
    ASSERT(winStats.games > 0);

    double wins = (color == PointColor::BLACK()) ? winStats.black_wins : winStats.white_wins;
    double value = wins / winStats.games;
    return value;
  }

  double getValueEstimate(PointColor color, Node* childNode, Move move) {
    WinStats childStats = childNode->winStats;
    WinStats amafStats = amafWinStats[move];

    double amaf_alpha = 1.0 - (childStats.games / (2*kRaveEquivalentPlayouts));

    if(amaf_alpha<0) amaf_alpha = 0;
    WinStats useStats;
    useStats.games = (1.0-amaf_alpha)*childStats.games + amaf_alpha*amafStats.games;
    useStats.black_wins = (1.0-amaf_alpha)*childStats.black_wins + amaf_alpha*amafStats.black_wins;
    useStats.white_wins = (1.0-amaf_alpha)*childStats.white_wins + amaf_alpha*amafStats.white_wins;

    return getValueEstimate(color, useStats);
  }

  double getUctWeight(PointColor color, double logParentVisitCount, Node* childNode, Move move) {
    double vi = getValueEstimate(color, childNode, move);
    if(childNode->winStats.games == 0) {
      return 1e12;
    }
    return vi + kUctC * sqrt(logParentVisitCount / childNode->winStats.games);
  }

  struct TreewalkResult {
    BOARD board;
    std::list<Node*> visited_nodes;
    std::list<Move> visited_moves;
    bool isGameFinished;
  };

  TreewalkResult doUctTreewalk(const BOARD& b, PointColor color) {
    TreewalkResult result;
    result.board = b;

    Node* node = getNodeForBoard(result.board);

    bool use_modulo = true;

    while(true) {
      result.visited_nodes.push_back(node);

      result.isGameFinished = result.board.isGameFinished();
      if(result.isGameFinished || node->isLeafNode()) {
        return result;
      }

      double logParentVisitCount = log(node->winStats.games);

      //build the weighted choice for our child nodes
      uint moduloNumerator = 0;
      uint moduloDenominator = 1;
      if(use_modulo) {
        moduloNumerator = kModuloPlayoutsNumerator;
        moduloDenominator = kModuloPlayoutsDenominator;
        use_modulo = false;
      }
      std::vector<Move> moves;
      result.board.getValidMoves(color, moves, moduloNumerator, moduloDenominator);

      //subboard.dump();

      std::vector<double> weights(moves.size());
      double weights_sum = 0;
      for(uint i=0; i<moves.size(); i++) {
        double weight = 1e5; //weight of not-found nodes
        typename Node::ChildMap::iterator ci = node->children.find(moves[i]);
        Node* childNode = 0;
        if(ci != node->children.end()) {
          childNode = ci->second;
          weight = getUctWeight(color, logParentVisitCount, childNode, moves[i]);
        }
        //LOG("%s 0x%08x %.2f", moves[i].point.toGtpVertex(subboard.getSize()).c_str(), childNode, weight);
        weights[i] = weight;
        weights_sum += weight;
      }

      //choose a move
      int idx = mChooser->choose((uint)weights.size(), &weights[0], weights_sum);
      Move move;
      if(idx == -1) {
        move = Move(color, Point::pass());
      } else {
        move = moves[idx];
      }
      //LOG("%14.2f %d", weights_sum, idx);

      //make the move
      result.board.playMoveAssumeLegal(move);
      Node* childNode = getNodeForBoard(result.board);
      node->children[move] = childNode;
      node = childNode;
      color = color.enemy();
      result.visited_moves.push_back(move);
    }
  }

  void doTrace(const BOARD& _b, PointColor _color) {
    ASSERT(!_b.isGameFinished());

    //walk tree
    TreewalkResult twr = doUctTreewalk(_b, _color);

    const BOARD& playoutBoard = twr.board;
    const std::list<Node*>& visitedNodes = twr.visited_nodes;
    const std::list<Move>& visitedMoves = twr.visited_moves;
    PointColor playoutColor = playoutBoard.getWhosTurn();

    //do playout
    PlayoutResults pr;
    if(!twr.isGameFinished) {
      doRandomPlayouts(playoutBoard, kNumPlayoutsPerTrace, playoutColor, pr);
    } else {
      pr.games = 1;
      PointColor winner = twr.board.winner();
      if(winner == PointColor::BLACK()) {
        pr.black_wins = 1;
      } else if(winner == PointColor::WHITE()) {
        pr.white_wins = 1;
      }
    }

    ++total_traces;
    --countdown;

    //update visited nodes
    typename std::list<Node*>::const_reverse_iterator i1 = visitedNodes.rbegin();
    while(i1 != visitedNodes.rend()) {
      Node* node = *i1;
      ++i1;
      WinStats& winStats = node->winStats;

      winStats.games += pr.games;
      winStats.black_wins += pr.black_wins;
      winStats.white_wins += pr.white_wins;
    }

    //update visited moves
    typename std::list<Move>::const_iterator i2 = visitedMoves.begin();
    while(i2 != visitedMoves.end()) {
      Move m = *i2;
      ++i2;
      WinStats& winStats = amafWinStats[m];

      winStats.games += pr.games;
      winStats.black_wins += pr.black_wins;
      winStats.white_wins += pr.white_wins;
    }
    size_t& depth = moveMaxPlies[*visitedMoves.begin()];
    depth = std::max(depth, visitedMoves.size());
  }

  void step(const BOARD& _b, PointColor _color) {
    LOG("# step");
    for(uint i=0; i<kTracesPerGuiUpdate; i++) {
        doTrace(_b, _color);
    }
    gogui_info(_b, _color);
  }

  int getMaxTreeDepth(const BOARD& b, Move move) {
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

    std::vector<Move> moves;
    subboard.getValidMoves(move.color.enemy(), moves);

    int max = 0;
    for(uint i=0; i<moves.size(); i++) {
      max = std::max(max, getMaxTreeDepth(subboard, moves[i]));
    }

    return 1 + max;
  }

  static const int BAD_MIN=0;
  int getMinTreeDepth(const BOARD& b, Move move) {
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

    std::vector<Move> moves;
    subboard.getValidMoves(move.color.enemy(), moves);

    int min = 0;
    for(uint i=0; i<moves.size(); i++) {
      min = std::min(min, getMinTreeDepth(subboard, moves[i]));
    }

    return 1 + min;
  }

  void gogui_info(const BOARD& b, PointColor color) {
    std::vector<NodeValue> nodeValues;
    rankMoves(b, color, nodeValues);

    uint ct = cykill_millisTime();
    uint millis = ct - startTime;

    std::string text;
    if(kGuiShowMoves>0) {
      text += strprintf("SQUARE %s\n", get<2>(nodeValues[0]).toString().c_str());
      int moveCount = std::min(kGuiShowMoves, (uint)nodeValues.size());
      if(moveCount > 1) {
        text += "CIRCLE";
        for(uint i=1; (i<kGuiShowMoves) && (i<nodeValues.size()); i++) {
          text += ' ';
          text += get<2>(nodeValues[i]).toString();
        }
        text += '\n';
      }
    }

    Move bestMove = get<2>(nodeValues[0]);
    if(bestMove != countdownMove) {
      countdownMove = bestMove;
      countdown = kCountdownToCertainty;
    }

    uint min_visits = 1e6;
    for(uint i=0; (i<kGuiShowMoves) && (i<nodeValues.size()); i++) {
      Node* childNode = get<1>(nodeValues[i]);
      if(childNode) {
        min_visits = std::min(min_visits, childNode->winStats.games);
      }
    }

    gotMoveCertainty = 0;
    if(countdown < 0) {
      if(min_visits >= kMinVisitsForCertainty) {
        gotMoveCertainty = 1;
      }
    }

    size_t maxPly = 0;
    for(typename MovePlyDepths::const_iterator i = moveMaxPlies.begin(); i != moveMaxPlies.end(); ++i) {
      maxPly = std::max(maxPly, i->second);
    }

    text += strprintf("TEXT %d traces %d/s -- countdown: %d -- min_visits: %d -- maxPly: %d -- search nodes: %d\n", total_traces, total_traces * 1000 / (millis+1), countdown, (int)min_visits, maxPly, all_nodes.size());

    //std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
    std::string gfx = "gogui-gfx:\n"+text+"\n\n";
    fputs(gfx.c_str(), stderr);
  }

  typedef tuple<double, Node*, Move> NodeValue;
  typedef double (Mcts2::*MoveValueFn)(const BOARD& b, PointColor color, Move move, Node* childNode);

  static bool compare(const NodeValue& a, const NodeValue& b) {
    return get<0>(a) > get<0>(b);
  }

  double visits_moveValue(const BOARD& b, PointColor color, Move move, Node* childNode) {
    return childNode->winStats.games;
  }
  double winrate_moveValue(const BOARD& b, PointColor color, Move move, Node* childNode) {
    double wincount = (color == PointColor::WHITE()) ? childNode->winStats.white_wins : childNode->winStats.black_wins;
    double winrate = wincount / childNode->winStats.games;
    return winrate;
  }
  double minimizeResponseWinRate_moveValue(const BOARD& b, PointColor color, Move move, Node* childNode) {
    BOARD subboard(b);
    subboard.playMoveAssumeLegal(move);

    std::vector<NodeValue> counterValues;
    rankMoves(subboard, color.enemy(), counterValues, &Mcts2<BOARD>::winrate_moveValue);

    double result = 0;
    if(!counterValues.empty()) {
      result = 1.0 - get<0>(counterValues[0]);
    }
    return result;
  }

  MoveValueFn getMoveValueFn() {
    //return &Mcts2<BOARD>::visits_moveValue;
    return &Mcts2<BOARD>::winrate_moveValue;
    //return &Mcts2<BOARD>::minimizeResponseWinRate_moveValue;
  }

  void rankMoves(const BOARD& b, PointColor color, std::vector<NodeValue>& nodeValues) {
    rankMoves(b, color, nodeValues, getMoveValueFn());
  }

  void rankMoves(const BOARD& b, PointColor color, std::vector<NodeValue>& nodeValues, MoveValueFn moveValueFn) {
    std::vector<Move> moves;
    b.getValidMoves(color, moves);

    Node* rootNode = getNodeForBoard(b);

    nodeValues.clear();
    nodeValues.reserve(moves.size());

    for(uint i=0; i<moves.size(); i++) {
      typename Node::ChildMap::iterator ci = rootNode->children.find(moves[i]);
      if(ci != rootNode->children.end()) {
        Node* childNode = ci->second;
        double value = (*this.*moveValueFn)(b, color, moves[i], childNode);
        nodeValues.push_back(make_tuple(value, childNode, moves[i]));
      }
    }

    std::sort(nodeValues.begin(), nodeValues.end(), compare);
  }

  static void logNodeValues(const BOARD& b, const std::vector<NodeValue>& nodeValues) {
    for(uint i=0; i<nodeValues.size(); i++) {
      const NodeValue& nv = nodeValues[i];
      LOG("%.2f %08x %2s", get<0>(nv), get<1>(nv), get<2>(nv).toString().c_str());
    }
  }

  Move getBestMove(const BOARD& b, PointColor color) {
    std::vector<NodeValue> nodeValues;
    rankMoves(b, color, nodeValues);

    Node* rootNode = getNodeForBoard(b);
    double logParentVisitCount = log(rootNode->winStats.games);

    for(uint i=0; i<nodeValues.size(); i++) {
      double value = get<0>(nodeValues[i]);
      Node* childNode = get<1>(nodeValues[i]);
      Move move = get<2>(nodeValues[i]);
      double uct_weight = getUctWeight(color, logParentVisitCount, childNode, move);

      LOG("move candidate: %2s maxPly:%2d visits: %6d b:%6d w:%6d t:%6d black_winrate: %.6f value: %.6f uct_weight: %.6f",
          move.toString().c_str(),
          moveMaxPlies[move],
          childNode->winStats.games,
          childNode->winStats.black_wins,
          childNode->winStats.white_wins,
          childNode->winStats.ties(),
          childNode->winStats.black_winrate(),
          value,
          uct_weight
         );

      BOARD subboard(b);
      subboard.playMoveAssumeLegal(move);
      Node* c_rootNode = getNodeForBoard(subboard);

      double c_logParentVisitCount = log(c_rootNode->winStats.games);

      std::vector<NodeValue> counterValues;
      rankMoves(subboard, color.enemy(), counterValues, &Mcts2<BOARD>::winrate_moveValue);
      for(uint j=0; j<std::min(4, (int)counterValues.size()); j++) {
        double c_value = get<0>(counterValues[j]);
        Node* c_childNode = get<1>(counterValues[j]);
        Move c_move = get<2>(counterValues[j]);
        double c_uct_weight = getUctWeight(color.enemy(), c_logParentVisitCount, c_childNode, c_move);

        LOG("    counter: %2s visits: %6d b:%6d w:%6d t:%6d black_winrate:%.6f value: %.6f uct_weight: %.6f",
            c_move.toString().c_str(),
            c_childNode->winStats.games,
            c_childNode->winStats.black_wins,
            c_childNode->winStats.white_wins,
            c_childNode->winStats.ties(),
            c_childNode->winStats.black_winrate(),
            c_value,
            c_uct_weight
           );
      }
    }

    return get<2>(nodeValues[0]);
  }
};

}


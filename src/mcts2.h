#pragma once

namespace {

using boost::shared_ptr;
using namespace boost::tuples;

template<class BOARD>
struct Mcts2 {

  static const double kUtcBias = 0.2;
  static const double kTracesPerGuiUpdate = 2000;

  struct Node {
    uint64_t zobrist;
    uint num_visits;
    uint black_wins;

    typedef std::map<typename BOARD::Move, Node*> ChildMap;

    ChildMap children;

    Node(uint64_t zobrist) : zobrist(zobrist), num_visits(0), black_wins(0) {
    }

    bool isLeafNode() const {
      return num_visits == 0;
    }
  };

  typedef std::map<uint64_t, Node> AllNodesMap;
  AllNodesMap all_nodes;

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

  double getValueEstimate(Node* childNode, BoardState playerColor) {
    ASSERT(childNode->num_visits > 0);

    double wins = (playerColor == BoardState::BLACK()) ? childNode->black_wins : (childNode->num_visits - childNode->black_wins);
    double value = wins / childNode->num_visits;
    return value;
  }

  double getUctWeight(double logParentVisitCount, Node* childNode, BoardState playerColor) {
    double vi = getValueEstimate(childNode, playerColor);
    if(childNode->num_visits == 0) {
      return vi;
    }
    return vi + kUtcBias * sqrt(logParentVisitCount / childNode->num_visits);
  }

  typedef tuple<BOARD, std::list<Node*> > TreewalkResult;

  TreewalkResult doUctTreewalk(const BOARD& b, BoardState playerColor) {
    BOARD subboard(b);
    std::list<Node*> visited_nodes;

    Node* node = getNodeForBoard(subboard);

    while(true) {
      visited_nodes.push_back(node);

      if(node->isLeafNode()) {
        return TreewalkResult(subboard, visited_nodes);
      }

      double logParentVisitCount = log(node->num_visits);

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
          weight = getUctWeight(logParentVisitCount, childNode, playerColor);
        }
        //LOG("%s 0x%08x %.2f", moves[i].point.toGtpVertex(subboard.getSize()).c_str(), childNode, weight);
        weights[i] = weight;
        weights_sum += weight;
      }

      //choose a move
      int idx = mChooser->choose((uint)weights.size(), &weights[0], weights_sum);
      //LOG("%14.2f %d", weights_sum, idx);

      //make the move
      subboard.playMoveAssumeLegal(moves[idx]);
      Node* childNode = getNodeForBoard(subboard);
      node->children[moves[idx]] = childNode;
      node = childNode;
      playerColor = playerColor.enemy();
    }
  }

  void doTrace(const BOARD& _b, BoardState _playerColor, RandomPlayerPtr randomPlayer) {
    //walk tree
    TreewalkResult twr = doUctTreewalk(_b, _playerColor);

    const Board& playoutBoard = get<0>(twr);
    const std::list<Node*> visitedNodes = get<1>(twr);
    BoardState playoutColor = playoutBoard.getWhosTurn();

    //do playout
    PlayoutResults pr;
    randomPlayer->doPlayouts(playoutBoard, 1, playoutColor, pr);
    ++total_playouts;

    //update visited nodes
    typename std::list<Node*>::const_reverse_iterator i1 = visitedNodes.rbegin();
    while(i1 != visitedNodes.rend()) {
      Node* node = *i1;
      node->num_visits += (pr.black_wins + pr.white_wins);
      node->black_wins += pr.black_wins;
      ++i1;
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

    std::string text = strprintf("TEXT %d playouts %d/s\n", total_playouts, total_playouts * 1000 / millis);

    //std::string gfx = "gogui-gfx:\n"+var+"\n"+influence+"\n"+label+"\n"+text+"\n\n";
    std::string gfx = "gogui-gfx:\n"+text+"\n\n";
    fputs(gfx.c_str(), stderr);
  }

  Move getBestMove(const BOARD& b, BoardState playerColor) {
    std::vector<typename BOARD::Move> moves;
    b.getValidMoves(playerColor, moves);

    Move bestMove = moves[0];
    double v = -1e6;

    Node* rootNode = getNodeForBoard(b);

    double logParentVisitCount = log(rootNode->num_visits);

    for(uint i=0; i<moves.size(); i++) {
      typename Node::ChildMap::iterator ci = rootNode->children.find(moves[i]);
      if(ci != rootNode->children.end()) {
        Node* childNode = ci->second;
        LOG("move candidate: %s visits: %d black_wins: %d value: %.2f",
            moves[i].point.toGtpVertex(b.getSize()).c_str(),
            childNode->num_visits,
            childNode->black_wins,
            getUctWeight(logParentVisitCount, childNode, playerColor)
           );
        double thisv = childNode->num_visits;
        if(thisv > v) {
          bestMove = moves[i];
          v = thisv;
        }
      }
    }

    return bestMove;
  }
};

}


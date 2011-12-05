#pragma once

template<class BOARD>
struct Mcts2 {

  struct Node {
    uint64_t zobrist;
    uint num_visits;
    uint black_wins;

    std::map<Board::Move, Node*> children;

    Node(uint64_t zobrist) : zobrist(zobrist), num_visits(0), black_wins(0) {
    }
  };

  std::map<uint64_t, Node> all_nodes;

  Mcts2() {
  }

  Node* getNodeForBoard(const Board& b) {
    uint64_t zobrist = b.zobrist();
    std::map<Board::Move, Node*>::iterator i1 = all_nodes.find(zobrist);
    if(i1 == all_nodes.end()) {
      i1 = all_nodes.insert(new Node(zobrist));
    }
    return i1->second;
  }

  Node* do_uct_treewalk(const Board& b, BoardState playerColor) {
  }

};


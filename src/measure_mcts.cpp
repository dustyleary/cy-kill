#include "config.h"

class SimpleTreeGame {
  typedef uint Move;
  bool isBlacksTurn;
  std::vector<Move> blackMoves;
  std::vector<Move> whiteMoves;

  SimpleTreeGame() : isBlacksTurn(true) {
  }

  SimpleTreeGame(const SimpleTreeGame& other) {
      memcpy(this, &other, sizeof(SimpleTreeGame));
  }

  SimpleTreeGame& operator=(const SimpleTreeGame& other) {
      memcpy(this, &other, sizeof(SimpleTreeGame));
      return *this;
  }
};

int main(int argc, char** argv) {
}

#include "config.h"

const double SWING = 0.2; //difference between 'bad move' and 'good move'
const uint TREE_WIDTH = 360;
const uint TREE_DEPTH=20;

struct SyntheticGame {
  uint mChoiceCount;
  uint mGoodChoice;
  uint mMovesPerGame;

  std::vector<uint> mBlackMoves;
  std::vector<uint> mWhiteMoves;

  struct Move {
    PointColor color;
    uint choice;
    Move() : color(PointColor::EMPTY()), choice(0) {}
    Move(PointColor color, Point vertex) : color(color) {
      choice = vertex.y()*kMaxBoardSize + vertex.x();
    }
    Move(PointColor color, uint choice) : color(color), choice(choice) {
    }
    std::string toString() const { return strprintf("%d",choice); }
    bool operator==(const Move& r) const { return (color == r.color) && (choice == r.choice); }
    bool operator!=(const Move& r) const { return !operator==(r); }
    bool operator<(const Move& r) const {
        if(color.toUint() < r.color.toUint()) return true;
        if(color.toUint() > r.color.toUint()) return false;
        return choice < r.choice;
    }
  };

  SyntheticGame(uint choiceCount=TREE_WIDTH, uint goodChoice=17, uint movesPerGame=TREE_DEPTH)
    : mChoiceCount(choiceCount)
    , mGoodChoice(goodChoice)
    , mMovesPerGame(movesPerGame)
  {
  }

  void reset() {
    mBlackMoves.clear();
    mWhiteMoves.clear();
  }

  void dump() const {
    fprintf(stderr, "black[%3d]:", (int)mBlackMoves.size());
    for(uint i=0; i<mBlackMoves.size(); i++) { fprintf(stderr, " %3d", mBlackMoves[i]); }
    fprintf(stderr, "\n");

    fprintf(stderr, "white[%3d]:", (int)mWhiteMoves.size());
    for(uint i=0; i<mWhiteMoves.size(); i++) { fprintf(stderr, " %3d", mWhiteMoves[i]); }
    fprintf(stderr, "\n");
  }

  PointColor getWhosTurn() const {
    if(mBlackMoves.size() >= mWhiteMoves.size()) return PointColor::WHITE();
    return PointColor::BLACK();
  }

  PointColor winner() const {
    if(!isGameFinished()) return PointColor::EMPTY();
    double good_move_count = 0;
    for(uint i=0; i<mBlackMoves.size(); i++) {
      if(mBlackMoves[i] == mGoodChoice) { good_move_count++; }
    }
    double black_chance = 0.5-SWING/2 + SWING * (good_move_count / mBlackMoves.size());
    return genrand_res53() <= black_chance ? PointColor::BLACK() : PointColor::WHITE();
  }

  bool isGameFinished() const {
    return (mBlackMoves.size() + mWhiteMoves.size()) >= mMovesPerGame;
  }

  bool isValidMove(Move m) const {
    return true;
  }

  void getValidMoves(PointColor c, std::vector<Move>& out, uint m=0, uint d=1) const {
    out.clear();
    for(uint i=0; i<mChoiceCount; i++) {
      out.push_back(Move(c, i));
    }
  }

  void playMoveAssumeLegal(Move m) {
    if(m.color == PointColor::BLACK()) mBlackMoves.push_back(m.choice);
    else if(m.color == PointColor::WHITE()) mWhiteMoves.push_back(m.choice);
  }

  Move getRandomMove(PointColor c) const { return Move(c, gen_rand64() % mChoiceCount); }

  uint64_t zobrist() const {
    uint64_t result = 0;
    for(uint i=0; i<mBlackMoves.size(); i++) { result ^= Zobrist::black[Point::fromUint(mBlackMoves[i])]; }
    for(uint i=0; i<mWhiteMoves.size(); i++) { result ^= Zobrist::white[Point::fromUint(mWhiteMoves[i])]; }
    return result;
  }
};

namespace tbb {

template<>
class tbb_hash<SyntheticGame::Move> {
public:
    size_t operator()(const SyntheticGame::Move& t) const {
        return tbb_hasher(t.color.toUint()) ^ tbb_hasher(t.choice);
    }
};

}


class GtpSynthetic : public GtpMcts<SyntheticGame> {
public:
    GtpSynthetic() {
        uct_kRaveEquivalentPlayouts = 0.001;
        uct_kCountdownToCertainty = 1000000;
        uct_kMinVisitsForCertainty = 1000;
        max_traces = 8000000;
    }
};

int main(int argc, char** argv) {
    GtpSynthetic gtp;
    gtp.run(argc, argv);

    return 0;
}



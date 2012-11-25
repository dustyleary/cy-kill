#pragma once

template<typename GAME>
class OpeningBook {
public:
    typedef typename GAME::Move Move;

    struct BookMoveInfo {
        std::string moveType;
        Move move;
        int patternSize;
        int bookCount;
        int winCount;
        BookMoveInfo(const std::string& moveType, Move move, int patternSize, int bookCount, int winCount) : moveType(moveType), move(move), patternSize(patternSize), bookCount(bookCount), winCount(winCount) {}
        std::string toString() const {
            return strprintf(
                "%10s   %2d   %6d/%6d   %s",
                moveType.c_str(),
                patternSize,
                winCount,
                bookCount,
                move.toString().c_str()
            );
        }
    };


    virtual std::vector<BookMoveInfo> getBookMoves(const GAME& board, PointColor color) =0;
    virtual std::vector<BookMoveInfo> getInterestingMoves(const GAME& board, PointColor color) =0;
};


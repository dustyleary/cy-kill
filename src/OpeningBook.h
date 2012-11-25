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
        bool operator<(const BookMoveInfo& rhs) const {
            if(moveType < rhs.moveType) return true;
            if(moveType > rhs.moveType) return false;

            if(patternSize < rhs.patternSize) return true;
            if(patternSize > rhs.patternSize) return false;

            if(bookCount < rhs.bookCount) return true;
            if(bookCount > rhs.bookCount) return false;

            if(winCount < rhs.winCount) return true;
            if(winCount > rhs.winCount) return false;

            return false;
        }
    };


    virtual std::vector<BookMoveInfo> getBookMoves(const GAME& board, PointColor color) =0;
    virtual std::vector<BookMoveInfo> getInterestingMoves_boardlocal(const GAME& board, PointColor color) =0;
    virtual std::vector<BookMoveInfo> getInterestingMoves_movelocal(const GAME& board, PointColor color) =0;
};


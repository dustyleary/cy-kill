#pragma once

class MysqlOpeningBook : public OpeningBook<Board> {
public:
    MysqlOpeningBook();

    std::vector<BookMoveInfo> getBookMoves_wholeboard(const Board& board, PointColor color);

    std::vector<BookMoveInfo> getInterestingMoves_boardlocal(const Board& board, PointColor color);
    std::vector<BookMoveInfo> getInterestingMoves_movelocal(const Board& board, PointColor color);

    BookMovesByType getBookMovesByType(const Board& board, PointColor color);

public:
    std::string connUrl;
    std::string connUser;
    std::string connPass;
    std::string connDb;

    sql::Driver* driver;
};



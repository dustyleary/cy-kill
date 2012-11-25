#pragma once

class MysqlOpeningBook : public OpeningBook<Board> {
public:
    MysqlOpeningBook();

    std::vector<BookMoveInfo> getBookMoves(const Board& board, PointColor color);
    std::vector<BookMoveInfo> getInterestingMoves(const Board& board, PointColor color);

private:
    std::string connUrl;
    std::string connUser;
    std::string connPass;
    std::string connDb;

    sql::Driver* driver;
};


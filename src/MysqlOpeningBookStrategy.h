#pragma once

class MysqlOpeningBookStrategy : public GameStrategy<Board> {
public:
    MysqlOpeningBookStrategy();

    Board::Move getMove(const Board& board, PointColor color);

private:
    std::string connUrl;
    std::string connUser;
    std::string connPass;
    std::string connDb;

    sql::Driver* driver;
};


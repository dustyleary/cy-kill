#include "config.h"

MysqlOpeningBookStrategy::MysqlOpeningBookStrategy() {
    driver = get_driver_instance();

    connUrl = "localhost";
    connUser = "root";
    connPass = "";
    connDb = "gobook";
}

template<uint N>
std::vector<Board::Move> getMovesThatMakePattern(const Board& board, PointColor color, const Pattern<N>& pattern, Point patternPoint) {
    std::vector<Board::Move> validMoves;
    std::vector<Board::Move> result;
    board.getValidMoves(color, validMoves);
    for(uint i=0; i<validMoves.size(); i++) {
        Board::Move candidate = validMoves[i];
        Board boardCopy = board;
        boardCopy.playMoveAssumeLegal(candidate);
        Pattern<N> p = boardCopy.canonicalPatternAt<N>(color, patternPoint);
        if(p == pattern) {
            result.push_back(candidate);
        }
    }
    return result;
}

struct BookMoveInfo {
    std::string patternType;
    Board::Move move;
    int bookCount;
    BookMoveInfo(const std::string& patternType, Board::Move move, int bookCount) : patternType(patternType), move(move), bookCount(bookCount) {}
};

template<uint N>
void getBookMovesForPoint(
    std::vector<BookMoveInfo>& result,
    boost::shared_ptr<sql::Connection> conn,
    const Board& board,
    const std::string& patternType,
    Point patternPoint,
    PointColor color
) {
    Pattern<N> p = board.canonicalPatternAt<N>(color, patternPoint);

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

    //std::string sql = strprintf("SELECT postPattern, count(*) AS num, SUM(win) AS win FROM %s WHERE prePattern='%s' GROUP BY postPattern HAVING num>1 AND win>1 ORDER BY num DESC", patternType.c_str(), p.toString().c_str());
    std::string sql = strprintf("SELECT postPattern, count(*) AS num, SUM(win) AS win FROM %s WHERE prePattern='%s' GROUP BY postPattern HAVING num>10 OR win>(num/2) ORDER BY num DESC", patternType.c_str(), p.toString().c_str());

    LOG("sql: %s", sql.c_str());
    stmt->execute(sql);
    std::auto_ptr< sql::ResultSet > res;
    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            Pattern<N> target_pattern = Pattern<N>::fromString(res->getString("postPattern"));
            std::vector<Board::Move> moves = getMovesThatMakePattern<N>(board, color, target_pattern, patternPoint);
            for(uint i=0; i<moves.size(); i++) {
                result.push_back(BookMoveInfo(patternType, moves[i], res->getInt("num")));
            }
        }
    } while (stmt->getMoreResults());
}

//std::vector<std::pair<Move, int> > getPatternMoves(
//    boost::shared_ptr<<sql::Connection> conn
//) {
//    std::auto_ptr<sql::Statement> stmt(con->createStatement());
//}

Board::Move MysqlOpeningBookStrategy::getMove(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    std::vector<BookMoveInfo> bookMoves;
    getBookMovesForPoint<19>(bookMoves, conn, board, "t19", COORD(9,9), color);

    if(board.lastMove.point != Point::invalid()) {
        Point starPoint = board.closestStarPoint(board.lastMove.point);
        LOG("closestStarPoint: %s", starPoint.toGtpVertex().c_str());
        std::string patternType;
        if(starPoint == COORD(9,9)) patternType = "t";
        else if(starPoint.x() == 9 || starPoint.y() == 9) patternType = "e";
        else patternType = "c";
#define doit(N) getBookMovesForPoint<N>(bookMoves, conn, board, patternType + #N, starPoint, color);
        doit(19);
        doit(17);
        doit(15);
        doit(13);
#undef doit
    }

    LOG("Book moves: %d", bookMoves.size());
    for(uint i=0; i<bookMoves.size(); i++) {
        BookMoveInfo& bmi = bookMoves[i];
        LOG("   %3s   %4d   %s", bmi.patternType.c_str(), bmi.bookCount, bmi.move.toString().c_str());
    }
    if(!bookMoves.empty()) {
        return bookMoves[0].move;
    }

    return Board::Move(color, Point::invalid());
}


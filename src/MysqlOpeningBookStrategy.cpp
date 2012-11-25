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
    std::string moveType;
    Board::Move move;
    int patternSize;
    int bookCount;
    int winCount;
    BookMoveInfo(const std::string& moveType, Board::Move move, int patternSize, int bookCount, int winCount) : moveType(moveType), move(move), patternSize(patternSize), bookCount(bookCount), winCount(winCount) {}
};

template<uint N>
void getBookMovesForPoint(
    std::vector<BookMoveInfo>& result,
    boost::shared_ptr<sql::Connection> conn,
    const Board& board,
    const std::string& moveType,
    Point patternPoint,
    PointColor color
) {
    std::string patternType;
    if(patternPoint == COORD(9,9)) patternType = strprintf("t%d", N);
    else if(patternPoint.x() == 9 || patternPoint.y() == 9) patternType = strprintf("e%d", N);
    else patternType = strprintf("c%d", N);

    Pattern<N> p = board.canonicalPatternAt<N>(color, patternPoint);

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

    //std::string sql = strprintf("SELECT postPattern, count(*) AS num, SUM(win) AS win FROM %s WHERE prePattern='%s' GROUP BY postPattern HAVING num>1 AND win>1 ORDER BY num DESC", patternType.c_str(), p.toString().c_str());
    std::string sql = strprintf("SELECT postPattern, num, win FROM boardlocal WHERE prePattern='%s' ORDER BY num DESC", p.toString().c_str());

    LOG("sql: %s", sql.c_str());
    stmt->execute(sql);
    std::auto_ptr< sql::ResultSet > res;
    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            Pattern<N> target_pattern = Pattern<N>::fromString(res->getString("postPattern"));
            std::vector<Board::Move> moves = getMovesThatMakePattern<N>(board, color, target_pattern, patternPoint);
            for(uint i=0; i<moves.size(); i++) {
                BookMoveInfo bmi = BookMoveInfo(moveType, moves[i], N, res->getInt("num"), res->getInt("win"));
                if(bmi.bookCount < 10) continue;
                result.push_back(bmi);
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
    getBookMovesForPoint<19>(bookMoves, conn, board, "wholeboard", COORD(9,9), color);

    if(board.lastMove.point != Point::invalid()) {
        Point starPoint = board.closestStarPoint(board.lastMove.point);
        LOG("closestStarPoint: %s", starPoint.toGtpVertex().c_str());
#define doit(N) getBookMovesForPoint<N>(bookMoves, conn, board, "response", starPoint, color);
        doit(19);
        doit(17);
        doit(15);
        doit(13);
        doit(11);
        doit(9);
#undef doit
    }

    for(uint x=3; x<19; x+=6) {
      for(uint y=3; y<19; y+=6) {
        Point pt = COORD(x,y);
#define doit(N) getBookMovesForPoint<N>(bookMoves, conn, board, "local", pt, color);
        doit(19);
        doit(17);
        doit(15);
        doit(13);
        doit(11);
        doit(9);
#undef doit
      }
    }

    LOG("Book moves: %d", bookMoves.size());
    for(uint i=0; i<bookMoves.size(); i++) {
        BookMoveInfo& bmi = bookMoves[i];
        LOG("   %10s   %2d   %6d/%6d   %s", bmi.moveType.c_str(), bmi.patternSize, bmi.winCount, bmi.bookCount, bmi.move.toString().c_str());
    }
    if(!bookMoves.empty()) {
        return bookMoves[0].move;
    }

    return Board::Move(PointColor::EMPTY(), Point::invalid());
}


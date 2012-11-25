#include "config.h"

typedef OpeningBook<Board>::BookMoveInfo BookMoveInfo;

MysqlOpeningBook::MysqlOpeningBook() {
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

    //std::string sql = strprintf("SELECT postPattern, count(*) AS num, SUM(win) AS win FROM %s WHERE prePattern='%s' GROUP BY postPattern HAVING num>1 AND win>1 ORDER BY num DESC", patternType.c_str(), p.toString().c_str());
    std::string sql = strprintf("SELECT postPattern, num, win FROM boardlocal WHERE num>=10 AND prePattern='%s'", p.toString().c_str());
    sql += " AND prepattern != ':0000002a:aaaaaaaa:aaaaaaaa:aaaaaaaa:aaaaaaaa:aaaaaaa0'"; //9x9 empty space
    sql += " ORDER BY num DESC";
    LOG("sql: %s", sql.c_str());

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

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

std::vector<BookMoveInfo> MysqlOpeningBook::getBookMoves(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    std::vector<BookMoveInfo> bookMoveInfos;
    getBookMovesForPoint<19>(bookMoveInfos, conn, board, "wholeboard", COORD(9,9), color);

    if(bookMoveInfos.empty()) {
        //only look for local responses if there are no whole board matches
        if(board.lastMove.point != Point::invalid()) {
            Point starPoint = board.closestStarPoint(board.lastMove.point);
#define doit(N) getBookMovesForPoint<N>(bookMoveInfos, conn, board, "response", starPoint, color);
            doit(15);
#undef doit
        }
    }

    for(uint i=0; i<bookMoveInfos.size(); i++) {
        BookMoveInfo& bmi = bookMoveInfos[i];
    }

    std::vector<BookMoveInfo> result;
    for(uint i=0; i<bookMoveInfos.size(); i++) {
        BookMoveInfo& bmi = bookMoveInfos[i];
        if(bmi.move.point == COORD(9,9)) continue; //skip tengen
        result.push_back(bmi);
        return result; //return just the most popular move
    }
    return result;
}

std::vector<BookMoveInfo> MysqlOpeningBook::getInterestingMoves_boardlocal(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    std::vector<BookMoveInfo> bookMoveInfos;

    for(uint x=3; x<19; x+=6) {
        for(uint y=3; y<19; y+=6) {
            Point pt = COORD(x,y);
#define doit(N) getBookMovesForPoint<N>(bookMoveInfos, conn, board, "boardlocal", pt, color);
            doit(9);
#undef doit
        }
    }

    std::sort(bookMoveInfos.rbegin(), bookMoveInfos.rend());
    return bookMoveInfos;
}

std::vector<BookMoveInfo> MysqlOpeningBook::getInterestingMoves_movelocal(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    std::vector<BookMoveInfo> bookMoveInfos;

    std::map<Point, Pat9> pointPatterns = board.getCanonicalPatternsForValidMoves<9>(color);
    std::set<Pat9> movelocal_patterns;

    for(std::map<Point,Pat9>::const_iterator i=pointPatterns.begin(); i!=pointPatterns.end(); ++i) {
        Pat9 pat9 = board.canonicalPatternAt<9>(color, i->first);
        movelocal_patterns.insert(pat9);
    }
    std::string sql = "SELECT id, prepattern, num FROM movelocal_patterns WHERE num>=10 AND prepattern IN (NULL";
    std::set<Pat9>::const_iterator i = movelocal_patterns.begin();
    while(i != movelocal_patterns.end()) {
        sql += strprintf(",'%s'", i->toString().c_str());
        ++i;
    }
    sql += ")";
    sql += " AND prepattern != ':0000002a:aaaaaaaa:aaaaaaaa:aaaaaaaa:aaaaaaaa:aaaaaaa0'"; //9x9 empty space

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

    stmt->execute(sql);
    std::auto_ptr< sql::ResultSet > res;
    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            int num = res->getInt("num");
            Pat9 pattern = Pat9::fromString(res->getString("prePattern"));
            for(std::map<Point,Pat9>::const_iterator i=pointPatterns.begin(); i!=pointPatterns.end(); ++i) {
                if(i->second == pattern) {
                    BookMoveInfo bmi = BookMoveInfo("movelocal", Move(color, i->first), 9, num, num);
                    bookMoveInfos.push_back(bmi);
                }
            }
        }
    } while (stmt->getMoreResults());

    std::sort(bookMoveInfos.rbegin(), bookMoveInfos.rend());
    return bookMoveInfos;
}


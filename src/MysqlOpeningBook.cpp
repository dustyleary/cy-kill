#include "config.h"

typedef OpeningBook<Board>::BookMoveInfo BookMoveInfo;
typedef OpeningBook<Board>::BookMovesByType BookMovesByType;

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
    Pattern<N> p = board.canonicalPatternAt<N>(color, patternPoint);

    std::string sql = strprintf("SELECT postPattern, num FROM boardlocal WHERE prePattern='%s'", p.toString().c_str());
    LOG("sql: %s", sql.c_str());

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

    stmt->execute(sql);
    std::auto_ptr< sql::ResultSet > res;
    int rowCount = 0;
    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            ++rowCount;
            int num = res->getInt("num");
            Pattern<N> target_pattern = Pattern<N>::fromString(res->getString("postPattern"));
            std::vector<Board::Move> moves = getMovesThatMakePattern<N>(board, color, target_pattern, patternPoint);
            for(uint i=0; i<moves.size(); i++) {
                BookMoveInfo bmi = BookMoveInfo(moveType, moves[i], N, num, num);
                result.push_back(bmi);
            }
        }
    } while (stmt->getMoreResults());
}

static const int RESPONSE_RADIUS = 11;

std::vector<BookMoveInfo> MysqlOpeningBook::getBookMoves_wholeboard(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    // wholeboard
    std::vector<BookMoveInfo> bookMoveInfos;
    getBookMovesForPoint<19>(bookMoveInfos, conn, board, "wholeboard", COORD(9,9), color);

    // response
    if((board.lastMove.point != Point::invalid()) && (board.lastMove.point != Point::pass())) {
        std::set<Point> examinedStarPoints;
        for(int dx=-RESPONSE_RADIUS/2; dx<RESPONSE_RADIUS/2; dx++) {
            for(int dy=-RESPONSE_RADIUS/2; dy<RESPONSE_RADIUS/2; dy++) {
                int x = dx + (int)board.lastMove.point.x();
                int y = dy + (int)board.lastMove.point.y();
                if(x>=0 && x<=board.getSize() && y>=0 && y<=board.getSize()) {
                    Point starPoint = board.closestStarPoint(COORD(x, y));
                    if(0 == examinedStarPoints.count(starPoint)) {
                        examinedStarPoints.insert(starPoint);
                        getBookMovesForPoint<RESPONSE_RADIUS>(bookMoveInfos, conn, board, "response", starPoint, color);
                    }
                }
            }
        }
    }

    // boardlocal
    for(uint x=3; x<19; x+=6) {
        for(uint y=3; y<19; y+=6) {
            Point pt = COORD(x,y);
#define doit(N) getBookMovesForPoint<N>(bookMoveInfos, conn, board, "boardlocal", pt, color);
            // larger areas are not necessary, they're redundant to smaller search areas
            // doit(19);
            // doit(17);
            // doit(15);
            // doit(13);
            doit(11);
            // doit(9);
#undef doit
        }
    }

    std::vector<BookMoveInfo> result;
    for(uint i=0; i<bookMoveInfos.size(); i++) {
        BookMoveInfo& bmi = bookMoveInfos[i];
        if(bmi.move.point == COORD(9,9)) continue; //skip tengen
        result.push_back(bmi);
    }
    return result;
}

std::vector<BookMoveInfo> MysqlOpeningBook::getInterestingMoves_movelocal(const Board& board, PointColor color) {
    boost::shared_ptr<sql::Connection> conn(driver->connect(connUrl, connUser, connPass));
    conn->setSchema(connDb);

    std::vector<BookMoveInfo> bookMoveInfos;

    std::map<Point, Pat9> pointPatterns = board.getCanonicalPatternsForValidMoves<9>(color);
    std::set<Pat9> movelocal_patterns;

    for(std::map<Point,Pat9>::const_iterator i=pointPatterns.begin(); i!=pointPatterns.end(); ++i) {
        Pat9 pat9 = board.canonicalPatternAt<9>(color, i->first);
        if(pat9.allPointsAreEmpty()) {
            continue; 
        }
        movelocal_patterns.insert(pat9);
    }
    std::string sql = "SELECT prePattern, num FROM movelocal WHERE prePattern IN (NULL";
    std::set<Pat9>::const_iterator i = movelocal_patterns.begin();
    while(i != movelocal_patterns.end()) {
        sql += strprintf(",'%s'", i->toString().c_str());
        ++i;
    }
    sql += ")";
    LOG("sql: %s", sql.c_str());

    std::auto_ptr<sql::Statement> stmt(conn->createStatement());

    stmt->execute(sql);
    std::auto_ptr< sql::ResultSet > res;
    do {
        res.reset(stmt->getResultSet());
        while (res->next()) {
            int num = res->getInt("num");
            Pat9 pattern = Pat9::fromString(res->getString("prePattern"));
            for(std::map<Point,Pat9>::const_iterator i=pointPatterns.begin(); i!=pointPatterns.end(); ++i) {
                if(i->second.canonical() == pattern.canonical()) {
                    BookMoveInfo bmi = BookMoveInfo("movelocal", Move(color, i->first), 9, num, num);
                    bookMoveInfos.push_back(bmi);
                    break; //dedup moves, only return each canonical pattern once
                }
            }
        }
    } while (stmt->getMoreResults());

    std::sort(bookMoveInfos.rbegin(), bookMoveInfos.rend());
    return bookMoveInfos;
}

void addBookMoveInfos(BookMovesByType& bmbt, const std::vector<BookMoveInfo>& bmis) {
    for(int i=0; i<bmis.size(); i++) {
        const BookMoveInfo& bmi = bmis[i];
        bmbt[bmi.moveType].push_back(bmi);
    }
}

BookMovesByType MysqlOpeningBook::getBookMovesByType(const Board& board, PointColor color) {
    BookMovesByType result;
    addBookMoveInfos(result, getBookMoves_wholeboard(board, color));
    return result;
}


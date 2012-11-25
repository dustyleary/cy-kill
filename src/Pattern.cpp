#include "config.h"

Pat3 invertedColorPat3s[PAT3_COUNT];
double pat3Gamma[PAT3_COUNT];

Pat3 getPat3InvertedColors(uint patternId) {
  return invertedColorPat3s[patternId];
}

double getPat3Gamma(Pat3 p) {
    return pat3Gamma[p.toUint()];
}

struct INIT {
    INIT() {
        for(uint patternId=0; patternId<PAT3_COUNT; patternId++) {
            Pat3 p = Pat3::fromUint(patternId);
            Pat3 inv = p._calculate_inverted_colors();
            invertedColorPat3s[patternId] = inv;

            pat3Gamma[patternId] = 0.001;
        }

        boost::shared_ptr<MysqlOpeningBook> book(new MysqlOpeningBook());
        boost::shared_ptr<sql::Connection> conn(book->driver->connect(book->connUrl, book->connUser, book->connPass));
        conn->setSchema(book->connDb);

        std::string sql = strprintf("SELECT pat3, gamma FROM pat3_gammas");

        std::auto_ptr<sql::Statement> stmt(conn->createStatement());
        stmt->execute(sql);
        std::auto_ptr< sql::ResultSet > res;
        do {
            res.reset(stmt->getResultSet());
            while (res->next()) {
                Pat3 pat = Pat3::fromString(res->getString("pat3"));
                double gamma = res->getDouble("gamma");
                pat3Gamma[pat.toUint()] = gamma;
            }
        } while (stmt->getMoreResults());
    }
} gINIT;


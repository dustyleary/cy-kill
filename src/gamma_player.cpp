#include "config.h"

/*

#define OK(expr) \
    if(SQLITE_OK != (expr)) { \
        throw std::runtime_error("sqlite error"); \
    }

double compute_weight(double max_gamma, double g) {
    double qa = pow(10.0, max_gamma / 400.0);
    double qb = pow(10.0, g / 400.0);
    double ea = qa / (qa + qb);
    double eb = qb / (qa + qb);
    return eb / ea;
}

void Gammas::load(const char* filename) {
    for(uint i=0; i<sizeof(gammas)/sizeof(gammas[0]); i++) {
        gammas[i] = 1.0;
    }
    return;
    memset(gammas, 0, sizeof(gammas));

    sqlite3* db;
    OK(sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, NULL));
    sqlite3_stmt* stmt;
    const char* sql = "select * from patterns order by gamma desc";
    OK(sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL));
    double max_gamma = 0;
    double min_gamma = 200;
    while(SQLITE_ROW == sqlite3_step(stmt)) {
        const unsigned char* pattern = sqlite3_column_text(stmt, 0);
        double gamma = sqlite3_column_double(stmt, 1);
        if(max_gamma == 0) {
            max_gamma = gamma;
        }
        if(gamma < min_gamma) {
            gamma = min_gamma;
        }
        Pat3 p = Pat3::fromString(std::string((const char*)pattern));
        double weight = compute_weight(max_gamma, gamma);
        gammas[p.toUint()] = weight;;
        //LOG("%s %f %f", pattern, gamma, weight);
    }
    OK(sqlite3_finalize(stmt));
    OK(sqlite3_close(db));

    //now do non-normalized gammas
    for(uint i=0; i<sizeof(gammas)/sizeof(gammas[0]); i++) {
        Pat3 p = Pat3::fromUint(i);
        gammas[p.toUint()] = gammas[p.canonical().toUint()];
    }
}

Gammas::Gammas() {
    load("gam3.dat");
}
*/

GammaFunc loadGammasFromFile(const char* filename) {
  return 0;
}

double fakeGammaFunc(uint patternId) {
  Pattern<3> p = Pattern<3>::fromUint(patternId);
  if(p.isSuicide()) {
    //fprintf(stderr, "fakeGamma got suicide pat\n");
    //p.dump();
    return 0.0;
  } else if(p.isSimpleEye()) {
    //fprintf(stderr, "fakeGamma got simpleEye pat\n");
    //p.dump();
    return 0.0;
  } else {
    return 1.0;
  }
}

GammaFunc fakeGammas() {
  return fakeGammaFunc;
}


#pragma once

template<uint kBoardSize>
struct Point : public Nat<Point<kBoardSize> > {
    PRIVATE_NAT_CONSTRUCTOR(Point);
    static const uint kBound = (kBoardSize+2)*(kBoardSize+1)+1;

    static Point fromCoord(int x, int y) { return Point((x+1) + (y+1)*(kBoardSize+1)); }
    uint x() const { return this->toUint() % (kBoardSize+1) - 1; }
    uint y() const { return this->toUint() / (kBoardSize+1) - 1; }

    Point N() { return Point(this->toUint() - (kBoardSize+1)); }
    Point S() { return Point(this->toUint() + (kBoardSize+1)); }
    Point E() { return Point(this->toUint() + 1); }
    Point W() { return Point(this->toUint() - 1); }

    static Point pass() { return fromCoord(-1,-1); }

    std::string toGtpVertex() const {
        if(*this == pass()) return "pass";
        char c1 = 'A'+x();
        if(c1>='I') c1++;
        char buf[16];
        sprintf(buf, "%c%d", c1, kBoardSize - y());
        return std::string(buf);
    }
};

#define FOREACH_POINT_DIR(pt, D, block) { \
    Point D; \
    D = pt.N(); block \
    D = pt.S(); block \
    D = pt.E(); block \
    D = pt.W(); block \
}

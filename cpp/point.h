#pragma once

template<uint kBoardSize>
struct Point : public Nat<Point<kBoardSize> > {
    PRIVATE_NAT_CONSTRUCTOR(Point);
    static const uint kBound = (kBoardSize+2)*(kBoardSize+1)+1;

    static Point fromCoord(int x, int y) { return Point((x+1) + (y+1)*(kBoardSize+1)); }
    uint x() const { return toUint() % (kBoardSize+1) - 1; }
    uint y() const { return toUint() / (kBoardSize+1) - 1; }

    Point N() { return Point(toUint() - (kBoardSize+1)); }
    Point S() { return Point(toUint() + (kBoardSize+1)); }
    Point E() { return Point(toUint() + 1); }
    Point W() { return Point(toUint() - 1); }

    Point() : Nat(-1) {
        ASSERT(!isValid());
    }
};


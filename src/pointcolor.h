#pragma once

static const char* BOARD_STATE_CHARS = "XO.#";
struct PointColor : public Nat<PointColor> {
    PRIVATE_NAT_CONSTRUCTOR(PointColor);
    static const uint kBound = 4;

    static PointColor BLACK() { return PointColor(0); }
    static PointColor WHITE() { return PointColor(1); }
    static PointColor EMPTY() { return PointColor(2); }
    static PointColor WALL()  { return PointColor(3); }

    bool isPlayer() const { return (*this == BLACK()) || (*this == WHITE()); }

    PointColor enemy() const {
        ASSERT(*this == BLACK() || *this == WHITE());
        return PointColor(1 - toUint());
    }

    char stateChar() const { return BOARD_STATE_CHARS[toUint()]; }
};


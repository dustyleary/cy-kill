#pragma once

static const char* BOARD_STATE_CHARS = ".XO#";
struct BoardState : public Nat<BoardState> {
    PRIVATE_NAT_CONSTRUCTOR(BoardState);
    static const uint kBound = 4;

    BoardState() : Nat<BoardState>(0) {}
    static BoardState EMPTY() { return BoardState(0); }
    static BoardState BLACK() { return BoardState(1); }
    static BoardState WHITE() { return BoardState(2); }
    static BoardState WALL()  { return BoardState(3); }

    bool isPlayer() { return (*this == BLACK()) || (*this == WHITE()); }

    BoardState enemy() {
        ASSERT(*this == BLACK() || *this == WHITE());
        return BoardState(toUint() ^ 3);
    }

    char stateChar() const { return BOARD_STATE_CHARS[toUint()]; }
};


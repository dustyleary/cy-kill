#pragma once

template<typename GAME>
class OpeningBook {
public:
    virtual std::vector<typename GAME::Move> getBookMoves(const GAME& board, PointColor color) =0;
    virtual std::vector<typename GAME::Move> getInterestingMoves(const GAME& board, PointColor color) =0;
};


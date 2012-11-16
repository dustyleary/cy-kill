#pragma once

template<typename GAME>
class GameStrategy {
public:
    virtual typename GAME::Move getMove(const GAME& board, PointColor color) =0;
};


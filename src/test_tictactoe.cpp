#include "gtest/gtest.h"
#include "config.h"

namespace {
typedef TicTacToeGame::Move Move;

TEST(TicTacToeGame, winner) {
    TicTacToeGame fixture;

    fixture.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    fixture.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    fixture.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    fixture.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    EXPECT_EQ(PointColor::EMPTY(), fixture.winner());

    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
        EXPECT_EQ(PointColor::BLACK(), g.winner());
    }
    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
        EXPECT_EQ(PointColor::BLACK(), g.winner());
    }

    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,2));
        EXPECT_EQ(PointColor::BLACK(), g.winner());
    }
    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));
        EXPECT_EQ(PointColor::BLACK(), g.winner());
    }

    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,2));
        EXPECT_EQ(PointColor::BLACK(), g.winner());
    }

    if(true) {
        TicTacToeGame g = fixture;
        g.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
        g.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
        g.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));
        EXPECT_EQ(PointColor::WHITE(), g.winner());
    }
}

}


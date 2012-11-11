#include "gtest/gtest.h"
#include "config.h"

//these tests mainly exist to setup known states
//they rely on GammaPlayer calling its own assertGoodState

TEST(GammaPlayer, emptyBoardPointSet) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);
}

TEST(GammaPlayer, makeMove) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    gp.playMove(b, Move(PointColor::BLACK(), 1,0));
}

TEST(GammaPlayer, testSuicide) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    //setup a19 to be a suicide point (for white)...
    gp.playMove(b, Move(PointColor::BLACK(), 1,0));
    gp.playMove(b, Move(PointColor::BLACK(), 0,1));
}

TEST(GammaPlayer, testSimpleEyeIsZeroWeight) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    gp.playMove(b, Move(PointColor::BLACK(), 1,0));
    gp.playMove(b, Move(PointColor::BLACK(), 0,1));

    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));

    EXPECT_EQ(0, gp.getWeight(PointColor::WHITE(), COORD(0,0)));
    EXPECT_EQ(0, gp.getWeight(PointColor::BLACK(), COORD(0,0)));
}

TEST(GammaPlayer, testBiggerSuicide) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    //setup b19 to be a suicide point (for white) although a19 is a white stone
    gp.playMove(b, Move(PointColor::WHITE(), 0,0));
    gp.playMove(b, Move(PointColor::BLACK(), 2,0));
    gp.playMove(b, Move(PointColor::BLACK(), 0,1));
    gp.playMove(b, Move(PointColor::BLACK(), 1,1));
}

TEST(GammaPlayer, testKo) {
    Board b(19);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    //setup a19 to be a ko point
    gp.playMove(b, Move(PointColor::BLACK(), 1,0));
    gp.playMove(b, Move(PointColor::BLACK(), 0,1));

    gp.playMove(b, Move(PointColor::WHITE(), 1,1));
    gp.playMove(b, Move(PointColor::WHITE(), 2,0));
    gp.playMove(b, Move(PointColor::WHITE(), 0,0)); //ko capture

    gp.playMove(b, Move(PointColor::BLACK(), 5,5)); //clear ko
}

TEST(GammaPlayer, doesnt_return_suicide_moves) {
    Board b(2);
    GammaPlayer gp(fakeGammas());
    gp.resetStateForNewBoard(b);

    gp.playMove(b, Move(PointColor::BLACK(), 0,0));
    gp.playMove(b, Move(PointColor::BLACK(), 1,1));

    EXPECT_EQ(Point::pass(), gp.getRandomMove(b, PointColor::WHITE()).point);
}

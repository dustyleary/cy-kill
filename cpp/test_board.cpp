#include "gtest/gtest.h"
#include "config.h"

class Empty19: public testing::Test {
public:
    Board<19> b;
};

class Empty9: public testing::Test {
public:
    Board<9> b;
};

class Empty5: public testing::Test {
public:
    Board<5> b;
};

TEST_F(Empty19, size) {
    EXPECT_EQ(19, b.getSize());
}
TEST_F(Empty9, size) {
    EXPECT_EQ(9, b.getSize());
}

TEST(Board, sizes) {
    printf("sizeof(Board<19>): %d\n", sizeof(Board<19>));
    printf("sizeof(Board<13>): %d\n", sizeof(Board<13>));
    printf("sizeof(Board<9>): %d\n", sizeof(Board<9>));
    printf("\n");
    printf("sizeof(Board<19>::PointSet): %d\n", sizeof(Board<19>::PointSet));
    printf("sizeof(Board<13>::PointSet): %d\n", sizeof(Board<13>::PointSet));
    printf("sizeof(Board<9>::PointSet): %d\n", sizeof(Board<9>::PointSet));
    printf("\n");
    printf("sizeof(Board<19>::ChainInfo): %d\n", sizeof(Board<19>::ChainInfo));
    printf("sizeof(Board<13>::ChainInfo): %d\n", sizeof(Board<13>::ChainInfo));
    printf("sizeof(Board<9>::ChainInfo): %d\n", sizeof(Board<9>::ChainInfo));
}

TEST_F(Empty19, countLiberties) {
    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    EXPECT_EQ(2, b.countLiberties(b.COORD(0,0)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,0)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,0)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, sharedLiberties) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,2));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,3));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,3));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,3));
    EXPECT_EQ(13, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, enemyRemovesLiberties) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, joinGroups) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(3,1)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    EXPECT_EQ(8, b.countLiberties(b.COORD(1,1)));
    EXPECT_EQ(8, b.countLiberties(b.COORD(2,1)));
    EXPECT_EQ(8, b.countLiberties(b.COORD(3,1)));
}

TEST_F(Empty19, surroundingRemovesLiberties) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,1)));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(5, b.countLiberties(b.COORD(0,1)));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,2));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,2));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,2));

    EXPECT_EQ(6, b.countLiberties(b.COORD(2,0)));
    EXPECT_EQ(1, b.countLiberties(b.COORD(0,1)));
}

TEST_F(Empty19, isSuicide) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    EXPECT_EQ(true, b.isSuicide(BoardState::WHITE(), b.COORD(0,0)));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,2));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,2));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,2));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    EXPECT_EQ(1, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(0,0)));
}

TEST_F(Empty19, isSuicide2) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));

    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(1,0)));
    EXPECT_EQ(false, b.isSuicide(BoardState::BLACK(), b.COORD(1,0)));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(1,0)));
    EXPECT_EQ(true, b.isSuicide(BoardState::BLACK(), b.COORD(1,0)));
}

TEST_F(Empty19, killGroup1) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));

    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,0)));
}

TEST_F(Empty19, kill2Groups) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(4,0));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(4,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(5,0));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));

    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(10, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(5,0)));
}

TEST_F(Empty19, koPoint) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));
    EXPECT_EQ(false, b.koPoint.isValid());

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
    EXPECT_EQ(b.koPoint, b.COORD(2,0));
}

TEST_F(Empty19, reuseChainIds) {
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));

    for(int i=0; i<1e5; i++) {
        //take and retake ko a bunch of times
        b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
        b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    }
}

TEST_F(Empty19, isSimpleEye_corner) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));
}

TEST_F(Empty19, isSimpleEye_wall) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));
}

TEST_F(Empty19, isSimpleEye_open) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,3));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,2));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,3));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));
}

TEST_F(Empty5, mcgMoves) {
    Board<5>::PointSet ps;

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(25, ps.size());

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(24, ps.size());

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(23, ps.size());
}

TEST_F(Empty5, kill_reopens_emptyPoints) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(4,0));

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(16, ps.size());

    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(4,1));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(20, ps.size());
}

TEST_F(Empty5, koPoint_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(25, ps.size());

    b.koPoint = b.COORD(1,1);

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(24, ps.size());
}

TEST_F(Empty5, simpleEye_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));

    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST_F(Empty5, suicide_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));

    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST(Board3, no_valid_mcgMoves_returns_empty) {
    Board<3> b;
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,2));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.assertGoodState();

    Board<3>::PointSet ps;
    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(0, ps.size());
}

TEST(Board2, crash_1) {
    Board<2> b;
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,0));
    b.assertGoodState();
    b.makeMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.assertGoodState();
}


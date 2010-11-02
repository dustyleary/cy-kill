#include "gtest/gtest.h"
#include "board.h"

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
/*
TEST_F(Empty19, size) {
    EXPECT_EQ(19, b.getSize());
}
TEST_F(Empty9, size) {
    EXPECT_EQ(9, b.getSize());
}

TEST(Enemy, simple) {
    EXPECT_EQ(ENEMY(BLACK), WHITE);
    EXPECT_EQ(ENEMY(WHITE), BLACK);
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
    printf("sizeof(Board<19>::Chain): %d\n", sizeof(Board<19>::Chain));
    printf("sizeof(Board<13>::Chain): %d\n", sizeof(Board<13>::Chain));
    printf("sizeof(Board<9>::Chain): %d\n", sizeof(Board<9>::Chain));
}

TEST_F(Empty19, countLiberties) {
    EXPECT_EQ(0, b.countLiberties(POS(0,0)));

    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    EXPECT_EQ(2, b.countLiberties(POS(0,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    EXPECT_EQ(3, b.countLiberties(POS(0,0)));
    EXPECT_EQ(3, b.countLiberties(POS(1,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    EXPECT_EQ(4, b.countLiberties(POS(0,0)));
    EXPECT_EQ(4, b.countLiberties(POS(1,0)));
    EXPECT_EQ(4, b.countLiberties(POS(1,1)));
}

TEST_F(Empty19, sharedLiberties) {
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(2,1));
    b.makeMoveAssumeLegal(BLACK, POS(3,1));
    b.makeMoveAssumeLegal(BLACK, POS(1,2));
    b.makeMoveAssumeLegal(BLACK, POS(3,2));
    b.makeMoveAssumeLegal(BLACK, POS(1,3));
    b.makeMoveAssumeLegal(BLACK, POS(2,3));
    b.makeMoveAssumeLegal(BLACK, POS(3,3));
    EXPECT_EQ(13, b.countLiberties(POS(1,1)));
}

TEST_F(Empty19, enemyRemovesLiberties) {
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    EXPECT_EQ(4, b.countLiberties(POS(1,1)));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    EXPECT_EQ(3, b.countLiberties(POS(1,1)));
}

TEST_F(Empty19, joinGroups) {
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(3,1));
    EXPECT_EQ(4, b.countLiberties(POS(1,1)));
    EXPECT_EQ(4, b.countLiberties(POS(3,1)));

    b.makeMoveAssumeLegal(BLACK, POS(2,1));
    EXPECT_EQ(8, b.countLiberties(POS(1,1)));
    EXPECT_EQ(8, b.countLiberties(POS(2,1)));
    EXPECT_EQ(8, b.countLiberties(POS(3,1)));
}

TEST_F(Empty19, surroundingRemovesLiberties) {
    b.makeMoveAssumeLegal(BLACK, POS(0,1));
    EXPECT_EQ(3, b.countLiberties(POS(0,1)));
    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    EXPECT_EQ(5, b.countLiberties(POS(0,1)));

    b.makeMoveAssumeLegal(WHITE, POS(2,0));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(2,2));
    b.makeMoveAssumeLegal(WHITE, POS(1,2));
    b.makeMoveAssumeLegal(WHITE, POS(0,2));

    EXPECT_EQ(6, b.countLiberties(POS(2,0)));
    EXPECT_EQ(1, b.countLiberties(POS(0,1)));
}

TEST_F(Empty19, isSuicide) {
    b.makeMoveAssumeLegal(BLACK, POS(0,1));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    EXPECT_EQ(true, b.isSuicide(WHITE, POS(0,0)));
    b.makeMoveAssumeLegal(WHITE, POS(0,2));
    b.makeMoveAssumeLegal(WHITE, POS(1,2));
    b.makeMoveAssumeLegal(WHITE, POS(2,2));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(2,0));
    EXPECT_EQ(1, b.countLiberties(POS(0,1)));
    EXPECT_EQ(false, b.isSuicide(WHITE, POS(0,0)));
}

TEST_F(Empty19, isSuicide2) {
    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(2,0));

    b.makeMoveAssumeLegal(WHITE, POS(0,1));
    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));

    EXPECT_EQ(false, b.isSuicide(WHITE, POS(1,0)));
    EXPECT_EQ(false, b.isSuicide(BLACK, POS(1,0)));

    b.makeMoveAssumeLegal(WHITE, POS(3,0));
    EXPECT_EQ(false, b.isSuicide(WHITE, POS(1,0)));
    EXPECT_EQ(true, b.isSuicide(BLACK, POS(1,0)));
}

TEST_F(Empty19, killGroup1) {
    b.makeMoveAssumeLegal(BLACK, POS(0,0));

    b.makeMoveAssumeLegal(WHITE, POS(0,1));
    b.makeMoveAssumeLegal(WHITE, POS(1,0));

    EXPECT_EQ(0, b.countLiberties(POS(0,0)));
    EXPECT_EQ(3, b.countLiberties(POS(0,1)));
    EXPECT_EQ(3, b.countLiberties(POS(1,0)));
}

TEST_F(Empty19, kill2Groups) {
    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(3,0));
    b.makeMoveAssumeLegal(BLACK, POS(4,0));

    b.makeMoveAssumeLegal(WHITE, POS(0,1));
    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(3,1));
    b.makeMoveAssumeLegal(WHITE, POS(4,1));
    b.makeMoveAssumeLegal(WHITE, POS(5,0));
    b.makeMoveAssumeLegal(WHITE, POS(2,0));

    EXPECT_EQ(0, b.countLiberties(POS(0,0)));
    EXPECT_EQ(10, b.countLiberties(POS(0,1)));
    EXPECT_EQ(3, b.countLiberties(POS(5,0)));
}

TEST_F(Empty19, koPoint) {
    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(2,0));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(3,0));
    EXPECT_EQ(b.koPoint, POS(-1,-1));

    b.makeMoveAssumeLegal(WHITE, POS(1,0));
    EXPECT_EQ(b.koPoint, POS(2,0));
}

TEST_F(Empty19, reuseChainIds) {
    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(2,0));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(3,0));

    for(int i=0; i<1e5; i++) {
        //take and retake ko a bunch of times
        b.makeMoveAssumeLegal(WHITE, POS(1,0));
        b.makeMoveAssumeLegal(BLACK, POS(2,0));
    }
}

TEST_F(Empty19, isSimpleEye_corner) {
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(0,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(0,1));
    EXPECT_EQ(true, b.isSimpleEye(BLACK, POS(0,0)));

    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(0,0)));
}

TEST_F(Empty19, isSimpleEye_wall) {
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(2,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(2,1));
    b.makeMoveAssumeLegal(BLACK, POS(3,0));
    EXPECT_EQ(true, b.isSimpleEye(BLACK, POS(2,0)));

    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BLACK, POS(2,0)));
    b.makeMoveAssumeLegal(WHITE, POS(3,1));
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(2,0)));
}

TEST_F(Empty19, isSimpleEye_open) {
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(2,2)));

    b.makeMoveAssumeLegal(BLACK, POS(2,1));
    b.makeMoveAssumeLegal(BLACK, POS(2,3));
    b.makeMoveAssumeLegal(BLACK, POS(1,2));
    b.makeMoveAssumeLegal(BLACK, POS(3,2));
    EXPECT_EQ(true, b.isSimpleEye(BLACK, POS(2,2)));

    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BLACK, POS(2,2)));
    b.makeMoveAssumeLegal(WHITE, POS(3,3));
    EXPECT_EQ(false, b.isSimpleEye(BLACK, POS(2,2)));
}

TEST_F(Empty5, mcgMoves) {
    Board<5>::PointSet ps;

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(25, ps.size());

    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(24, ps.size());

    b.makeMoveAssumeLegal(WHITE, POS(3,1));
    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(23, ps.size());
}

TEST_F(Empty5, kill_reopens_emptyPoints) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(2,0));
    b.makeMoveAssumeLegal(BLACK, POS(3,0));
    b.makeMoveAssumeLegal(BLACK, POS(4,0));

    b.makeMoveAssumeLegal(WHITE, POS(0,1));
    b.makeMoveAssumeLegal(WHITE, POS(1,1));
    b.makeMoveAssumeLegal(WHITE, POS(2,1));
    b.makeMoveAssumeLegal(WHITE, POS(3,1));

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(16, ps.size());

    b.makeMoveAssumeLegal(WHITE, POS(4,1));

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(20, ps.size());
}

TEST_F(Empty5, koPoint_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(25, ps.size());

    b.koPoint = POS(1,1);

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(24, ps.size());
}

TEST_F(Empty5, simpleEye_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(0,1));

    b.mcgMoves(BLACK, ps);
    EXPECT_EQ(21, ps.size());
}

TEST_F(Empty5, suicide_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.makeMoveAssumeLegal(BLACK, POS(0,1));

    b.mcgMoves(WHITE, ps);
    EXPECT_EQ(21, ps.size());
}
*/
TEST(Board2x2, crash_1) {
    Board<2> b;
    b.makeMoveAssumeLegal(BLACK, POS(0,1));
    b.makeMoveAssumeLegal(WHITE, POS(0,0));
    b.makeMoveAssumeLegal(BLACK, POS(1,0));
}

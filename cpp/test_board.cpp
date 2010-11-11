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

template<typename BOARD>
void dumpSizes() {
    printf("sizeof(Board): %d\n", (int)sizeof(BOARD));
    printf("sizeof(BoardState): %d\n", (int)sizeof(BoardState));
    printf("sizeof(Board::Point): %d\n", (int)sizeof(typename BOARD::Point));
    printf("sizeof(Board::ChainInfo): %d\n", (int)sizeof(typename BOARD::ChainInfo));
    BOARD b;
    printf("sizeof(Board::states): %d\n", (int)sizeof(b.states));
    printf("sizeof(Board::chain_next_point): %d\n", (int)sizeof(b.chain_next_point));
    printf("sizeof(Board::chain_ids): %d\n", (int)sizeof(b.chain_ids));
    printf("sizeof(Board::chain_infos): %d\n", (int)sizeof(b.chain_infos));
}

TEST(Board, sizes) {
    printf("Board<19>\n");
    dumpSizes<Board<19> >();
    printf("Board<9>\n");
    dumpSizes<Board<9> >();
}

TEST_F(Empty19, countLiberties) {
    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    EXPECT_EQ(2, b.countLiberties(b.COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,0)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, sharedLiberties) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,3));
    EXPECT_EQ(13, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, enemyRemovesLiberties) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,1)));
}

TEST_F(Empty19, joinGroups) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,1));
    EXPECT_EQ(4, b.countLiberties(b.COORD(1,1)));
    EXPECT_EQ(4, b.countLiberties(b.COORD(3,1)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    EXPECT_EQ(8, b.countLiberties(b.COORD(1,1)));
    EXPECT_EQ(8, b.countLiberties(b.COORD(2,1)));
    EXPECT_EQ(8, b.countLiberties(b.COORD(3,1)));
}

TEST_F(Empty19, surroundingRemovesLiberties) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,1)));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    EXPECT_EQ(5, b.countLiberties(b.COORD(0,1)));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,2));

    EXPECT_EQ(6, b.countLiberties(b.COORD(2,0)));
    EXPECT_EQ(1, b.countLiberties(b.COORD(0,1)));
}

TEST_F(Empty19, isSuicide) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    EXPECT_EQ(true, b.isSuicide(BoardState::WHITE(), b.COORD(0,0)));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    EXPECT_EQ(1, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(0,0)));
}

TEST_F(Empty19, isSuicide2) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));

    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(1,0)));
    EXPECT_EQ(false, b.isSuicide(BoardState::BLACK(), b.COORD(1,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), b.COORD(1,0)));
    EXPECT_EQ(true, b.isSuicide(BoardState::BLACK(), b.COORD(1,0)));
}

TEST_F(Empty19, killGroup1) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));

    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(1,0)));
}

TEST_F(Empty19, kill2Groups) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(4,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(4,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(5,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));

    EXPECT_EQ(0, b.countLiberties(b.COORD(0,0)));
    EXPECT_EQ(10, b.countLiberties(b.COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(b.COORD(5,0)));
}

TEST_F(Empty19, koPoint) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));
    EXPECT_EQ(false, b.koPoint.isValid());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
    EXPECT_EQ(b.koPoint, b.COORD(2,0));
}

TEST_F(Empty19, reuseChainIds) {
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,0));

    for(int i=0; i<100; i++) {
        //take and retake ko a bunch of times
        b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
        b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    }
}

TEST_F(Empty19, isSimpleEye_corner) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));
}

TEST_F(Empty19, isSimpleEye_wall) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,0)));
}

TEST_F(Empty19, isSimpleEye_open) {
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,2));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,3));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), b.COORD(2,2)));
}

TEST_F(Empty5, mcgMoves) {
    Board<5>::PointSet ps;

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(25, ps.size());

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(24, ps.size());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(23, ps.size());
}

TEST_F(Empty5, kill_reopens_emptyPoints) {
    Board<5>::PointSet ps;

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(4,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(3,1));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(16, ps.size());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(4,1));

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

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));

    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), b.COORD(0,0)));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST_F(Empty5, suicide_isnt_valid_mcgMove) {
    Board<5>::PointSet ps;

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,1));

    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST(Board3, no_valid_mcgMoves_returns_empty) {
    Board<3> b;
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));

    Board<3>::PointSet ps;
    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(0, ps.size());
}

TEST(Board3, bug1) {
    Board<3> b;
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,1));
    EXPECT_EQ(true, b.isInAtari(b.COORD(1,1)));
    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,1));
    EXPECT_EQ(BoardState::EMPTY(), b.bs(b.COORD(0,0)));
}

TEST(Board5, score_tt_1) {
    Board<5> b;
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(0,0));
    EXPECT_EQ(-25, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(1,0));
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(0,1));
    EXPECT_EQ(25, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(4,3));
    EXPECT_EQ(2, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(3,3));
    EXPECT_EQ(1, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), b.COORD(2,4));
    EXPECT_EQ(-2, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), b.COORD(4,4));
    EXPECT_EQ(1, b.trompTaylorScore());
}

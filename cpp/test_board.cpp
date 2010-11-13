#include "gtest/gtest.h"
#include "config.h"


TEST(Empty19, size) {
    Board b(19);
    EXPECT_EQ(19, b.getSize());
}
TEST(Empty9, size) {
    Board b(9);
    EXPECT_EQ(9, b.getSize());
}

TEST(Board, sizes) {
    printf("sizeof(Board): %d\n", (int)sizeof(Board));
    printf("sizeof(BoardState): %d\n", (int)sizeof(BoardState));
    printf("sizeof(Point): %d\n", (int)sizeof(Point));
    printf("sizeof(ChainInfo): %d\n", (int)sizeof(ChainInfo));
    Board b(19);
    printf("sizeof(Board::states): %d\n", (int)sizeof(b.states));
    printf("sizeof(Board::chain_next_point): %d\n", (int)sizeof(b.chain_next_point));
    printf("sizeof(Board::chain_ids): %d\n", (int)sizeof(b.chain_ids));
    printf("sizeof(Board::chain_infos): %d\n", (int)sizeof(b.chain_infos));
}

TEST(Empty, countLiberties) {
    Board b(19);
    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    EXPECT_EQ(2, b.countLiberties(COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    EXPECT_EQ(3, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(COORD(1,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(4, b.countLiberties(COORD(1,0)));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
}

TEST(Empty, sharedLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,3));
    EXPECT_EQ(13, b.countLiberties(COORD(1,1)));
}

TEST(Empty, enemyRemovesLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    EXPECT_EQ(3, b.countLiberties(COORD(1,1)));
}

TEST(Empty, joinGroups) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,1));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
    EXPECT_EQ(4, b.countLiberties(COORD(3,1)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,1));
    EXPECT_EQ(8, b.countLiberties(COORD(1,1)));
    EXPECT_EQ(8, b.countLiberties(COORD(2,1)));
    EXPECT_EQ(8, b.countLiberties(COORD(3,1)));
}

TEST(Empty, surroundingRemovesLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,1));
    EXPECT_EQ(3, b.countLiberties(COORD(0,1)));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    EXPECT_EQ(5, b.countLiberties(COORD(0,1)));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,2));

    EXPECT_EQ(6, b.countLiberties(COORD(2,0)));
    EXPECT_EQ(1, b.countLiberties(COORD(0,1)));
}

TEST(Empty, isSuicide) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    EXPECT_EQ(true, b.isSuicide(BoardState::WHITE(), COORD(0,0)));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,0));
    EXPECT_EQ(1, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), COORD(0,0)));
}

TEST(Empty, isSuicide2) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));

    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), COORD(1,0)));
    EXPECT_EQ(false, b.isSuicide(BoardState::BLACK(), COORD(1,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,0));
    EXPECT_EQ(false, b.isSuicide(BoardState::WHITE(), COORD(1,0)));
    EXPECT_EQ(true, b.isSuicide(BoardState::BLACK(), COORD(1,0)));
}

TEST(Empty, killGroup1) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,0));

    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(COORD(1,0)));
}

TEST(Empty, kill2Groups) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(4,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(4,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(5,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,0));

    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(10, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(COORD(5,0)));
}

TEST(Empty, koPoint) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,0));
    EXPECT_EQ(false, b.koPoint.isValid());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,0));
    EXPECT_EQ(b.koPoint, COORD(2,0));
}

TEST(Empty, reuseChainIds) {
    Board b(19);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,0));

    for(int i=0; i<100; i++) {
        //take and retake ko a bunch of times
        b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,0));
        b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));
    }
}

TEST(Empty, isSimpleEye_corner) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(0,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(0,0)));
}

TEST(Empty, isSimpleEye_wall) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(2,0)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,0));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(2,0)));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(2,0)));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,1));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(2,0)));
}

TEST(Empty, isSimpleEye_open) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(2,2)));

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,3));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,2));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(2,2)));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(2,2)));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,3));
    EXPECT_EQ(false, b.isSimpleEye(BoardState::BLACK(), COORD(2,2)));
}

TEST(Empty, mcgMoves) {
    Board b(5);
    PointSet ps;

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(25, ps.size());

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(24, ps.size());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,1));
    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(23, ps.size());
}

TEST(Empty, kill_reopens_emptyPoints) {
    Board b(5);
    PointSet ps;

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(4,0));

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,1));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(16, ps.size());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(4,1));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(20, ps.size());
}

TEST(Empty, koPoint_isnt_valid_mcgMove) {
    Board b(5);
    PointSet ps;

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(25, ps.size());

    b.koPoint = COORD(1,1);

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(24, ps.size());
}

TEST(Empty, simpleEye_isnt_valid_mcgMove) {
    Board b(5);
    PointSet ps;

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,1));

    EXPECT_EQ(true, b.isSimpleEye(BoardState::BLACK(), COORD(0,0)));

    b.mcgMoves(BoardState::BLACK(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST(Empty, suicide_isnt_valid_mcgMove) {
    Board b(5);
    PointSet ps;

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,1));

    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(21, ps.size());
}

TEST(Board3, no_valid_mcgMoves_returns_empty) {
    Board b(3);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,1));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(2,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,2));

    PointSet ps;
    b.mcgMoves(BoardState::WHITE(), ps);
    EXPECT_EQ(0, ps.size());
}

TEST(Board3, bug1) {
    Board b(3);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,0));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,0));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,1));
    EXPECT_EQ(true, b.isInAtari(COORD(1,1)));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,1));
    EXPECT_EQ(BoardState::EMPTY(), b.bs(COORD(0,0)));
}

TEST(Board5, score_tt_1) {
    Board b(5);
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(0,0));
    EXPECT_EQ(-25, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(1,0));
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(0,1));
    EXPECT_EQ(25, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(4,3));
    EXPECT_EQ(2, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(3,3));
    EXPECT_EQ(1, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,4));
    EXPECT_EQ(-2, b.trompTaylorScore());

    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(4,4));
    EXPECT_EQ(1, b.trompTaylorScore());
}

TEST(Board5, emptyPoints_bug) {
    Board b(5);
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(2,2));
    b.playMoveAssumeLegal(BoardState::WHITE(), COORD(3,2));
    b.playMoveAssumeLegal(BoardState::BLACK(), COORD(1,2));
    b.dump();
    EXPECT_EQ(22, b.emptyPoints.size());
    for(uint i=0; i<b.emptyPoints.size(); i++) {
        Point p = b.emptyPoints[i];
        EXPECT_EQ(BoardState::EMPTY(), b.bs(p));
        fprintf(stderr, "%s\n", p.toGtpVertex(b.getSize()).c_str());
    }
}

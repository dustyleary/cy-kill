#include "gtest/gtest.h"
#include "config.h"

typedef TwoPlayerGridGame::Move Move;
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
    printf("sizeof(PointColor): %d\n", (int)sizeof(PointColor));
    printf("sizeof(Point): %d\n", (int)sizeof(Point));
    printf("sizeof(ChainInfo): %d\n", (int)sizeof(ChainInfo));
    Board b(19);
    printf("sizeof(Board::states): %d\n", (int)sizeof(b.states));
    printf("sizeof(Board::chain_next_point): %d\n", (int)sizeof(b.chain_next_point));
    printf("sizeof(Board::chain_ids): %d\n", (int)sizeof(b.chain_ids));
    printf("sizeof(Board::chain_infos): %d\n", (int)sizeof(b.chain_infos));
}

TEST(Pattern, sizes) {
  printf("sizeof(Pattern<3>): %d\n", (int)sizeof(Pattern<3>));
  printf("sizeof(Pattern<5>): %d\n", (int)sizeof(Pattern<5>));
  printf("sizeof(Pattern<7>): %d\n", (int)sizeof(Pattern<7>));
  printf("sizeof(Pattern<9>): %d\n", (int)sizeof(Pattern<9>));
  printf("sizeof(Pattern<11>): %d\n", (int)sizeof(Pattern<11>));
  printf("sizeof(Pattern<13>): %d\n", (int)sizeof(Pattern<13>));
  printf("sizeof(Pattern<15>): %d\n", (int)sizeof(Pattern<15>));
  printf("sizeof(Pattern<17>): %d\n", (int)sizeof(Pattern<17>));
  printf("sizeof(Pattern<19>): %d\n", (int)sizeof(Pattern<19>));
}

TEST(Empty, countLiberties) {
    Board b(19);
    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    EXPECT_EQ(2, b.countLiberties(COORD(0,0)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    EXPECT_EQ(3, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(COORD(1,0)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    EXPECT_EQ(4, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(4, b.countLiberties(COORD(1,0)));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
}

TEST(Empty, sharedLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,2));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,3));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,3));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    EXPECT_EQ(13, b.countLiberties(COORD(1,1)));
}

TEST(Empty, enemyRemovesLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    EXPECT_EQ(3, b.countLiberties(COORD(1,1)));
}

TEST(Empty, joinGroups) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,1));
    EXPECT_EQ(4, b.countLiberties(COORD(1,1)));
    EXPECT_EQ(4, b.countLiberties(COORD(3,1)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    EXPECT_EQ(8, b.countLiberties(COORD(1,1)));
    EXPECT_EQ(8, b.countLiberties(COORD(2,1)));
    EXPECT_EQ(8, b.countLiberties(COORD(3,1)));
}

TEST(Empty, surroundingRemovesLiberties) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    EXPECT_EQ(3, b.countLiberties(COORD(0,1)));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    EXPECT_EQ(5, b.countLiberties(COORD(0,1)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));

    EXPECT_EQ(6, b.countLiberties(COORD(2,0)));
    EXPECT_EQ(1, b.countLiberties(COORD(0,1)));
}

TEST(Empty, isSuicide) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    EXPECT_EQ(true, b.isSuicide(PointColor::WHITE(), COORD(0,0)));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));
    EXPECT_EQ(1, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(false, b.isSuicide(PointColor::WHITE(), COORD(0,0)));
}

TEST(Empty, isSuicide2) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));

    EXPECT_EQ(false, b.isSuicide(PointColor::WHITE(), COORD(1,0)));
    EXPECT_EQ(false, b.isSuicide(PointColor::BLACK(), COORD(1,0)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));
    EXPECT_EQ(false, b.isSuicide(PointColor::WHITE(), COORD(1,0)));
    EXPECT_EQ(true, b.isSuicide(PointColor::BLACK(), COORD(1,0)));
}

TEST(Empty, killGroup1) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,0));

    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(3, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(COORD(1,0)));
}

TEST(Empty, kill2Groups) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 4,0));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 4,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 5,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));

    EXPECT_EQ(0, b.countLiberties(COORD(0,0)));
    EXPECT_EQ(10, b.countLiberties(COORD(0,1)));
    EXPECT_EQ(3, b.countLiberties(COORD(5,0)));
}

TEST(Empty, koPoint) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));
    EXPECT_EQ(false, b.koPoint.isValid());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,0));
    EXPECT_EQ(b.koPoint, COORD(2,0));
}

TEST(Empty, isSimpleEye_corner) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));
}

TEST(Empty, isSimpleEye_wall) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 2,0)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,0));
    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 2,0)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 2,0)));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 2,0)));
}

TEST(Empty, isSimpleEye_open) {
    Board b(19);
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 2,2)));

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,3));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,2));
    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 2,2)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 2,2)));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,3));
    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 2,2)));
}

TEST(Empty, isSimpleEye_atari) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
}

TEST(Empty, isSimpleEye_atari_for_both_colors) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    EXPECT_EQ(COORD(1,0), b.getAtariVertex(COORD(0,0)));
    EXPECT_EQ(COORD(1,0), b.getAtariVertex(COORD(2,0)));

    EXPECT_EQ(false, b.isSimpleEye(Move(PointColor::BLACK(), 1,0)));
}

TEST(Empty, getValidMoves) {
    Board b(5);
    std::vector<Move> moves;

    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(26, moves.size());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(25, moves.size());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(24, moves.size());
}

TEST(Empty, kill_reopens_emptyPoints) {
    Board b(5);
    std::vector<Move> moves;

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 4,0));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));

    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(17, moves.size());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 4,1));

    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(21, moves.size());
}

TEST(Empty, koPoint_isnt_valid_mcgMove) {
    Board b(5);
    std::vector<Move> moves;

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,2));

    //capture
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));

    //should not actually be ko
    EXPECT_EQ(Point::invalid(), b.koPoint);
}

TEST(Empty, snapback_is_not_real_ko) {
    Board b(5);
    std::vector<Move> moves;

    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(26, moves.size());

    b.koPoint = COORD(1,1);

    b.getValidMoves(PointColor::BLACK(), moves);
    EXPECT_EQ(25, moves.size());
}

/* simple eye IS a valid move...

TEST(Empty, simpleEye_isnt_valid_mcgMove) {
    Board b(5);
    PointSet ps;

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));

    EXPECT_EQ(true, b.isSimpleEye(Move(PointColor::BLACK(), 0,0)));

    b.getValidMoves(PointColor::BLACK(), ps);
    EXPECT_EQ(22, ps.size());
}
*/

TEST(Empty, suicide_isnt_valid_mcgMove) {
    Board b(5);
    std::vector<Move> moves;

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));

    b.getValidMoves(PointColor::WHITE(), moves);
    EXPECT_EQ(22, moves.size());
}

TEST(Board3, no_valid_getValidMoves_returns_empty) {
    Board b(3);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));

    std::vector<Move> moves;
    b.getValidMoves(PointColor::WHITE(), moves);
    EXPECT_EQ(1, moves.size());
}

TEST(Board3, bug1) {
    Board b(3);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    EXPECT_EQ(true, b.isInAtari(COORD(1,1)));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));
    EXPECT_EQ(PointColor::EMPTY(), b.bs(COORD(0,0)));
}

TEST(Board5, score_tt_1) {
    Board b(5, 0);
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    EXPECT_EQ(-25, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,0));
    EXPECT_EQ(0, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,1));
    EXPECT_EQ(25, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 4,3));
    EXPECT_EQ(2, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    EXPECT_EQ(1, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,4));
    EXPECT_EQ(-2, b.trompTaylorScore());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 4,4));
    EXPECT_EQ(1, b.trompTaylorScore());
}

void assertUniqueZobrists(const std::vector<uint64_t>& v) {
  for(uint i=0; i<v.size(); i++) {
    for(uint j=i+1; j<v.size(); j++) {
      EXPECT_NE(v[i], v[j]);
    }
  }
}

TEST(Empty19, zobrist_invariants) {
    Board b(19);
    std::vector<uint64_t> zobrists;
    zobrists.push_back(b.zobrist());

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    zobrists.push_back(b.zobrist());
    assertUniqueZobrists(zobrists);

    b.reset();
    EXPECT_EQ(zobrists[0], b.zobrist());
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,0));
    zobrists.push_back(b.zobrist());
    assertUniqueZobrists(zobrists);
}

TEST(Empty19, zobrist_respects_ko) {
    Board b(19);
    std::vector<uint64_t> zobrists;

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));

    zobrists.push_back(b.zobrist());
    assertUniqueZobrists(zobrists);

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,0));

    zobrists.push_back(b.zobrist());
    assertUniqueZobrists(zobrists);
    b.dump();

    //we are now setup, in a ko spot, but with no ko captures

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0)); //take the ko
    zobrists.push_back(b.zobrist());
    b.dump();
    assertUniqueZobrists(zobrists);

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,0)); //retake the ko (not allowed actually, but...)
    zobrists.push_back(b.zobrist());

    //we should still be unique because we are in the same setup as before
    //but now we have ko point at A19
    b.dump();
    assertUniqueZobrists(zobrists);
}

TEST(Empty19, zobrist_respects_whosTurn) {
    Board b(19);
    std::vector<uint64_t> zobrists;

    ASSERT_EQ(PointColor::BLACK(), b.getWhosTurn());
    zobrists.push_back(b.zobrist());

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), Point::pass()));

    ASSERT_EQ(PointColor::WHITE(), b.getWhosTurn());
    zobrists.push_back(b.zobrist());

    assertUniqueZobrists(zobrists);
}

TEST(Empty19, _calculatePattern_empty) {
    Board b(19);
    Pat3 p = b._calculatePatternAt<3>(COORD(0,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::EMPTY());
    EXPECT_EQ(p.getColorAt(2,2), PointColor::EMPTY());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::EMPTY());
    uint n,s,e,w;
    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(0, s);
    EXPECT_EQ(0, e);
    EXPECT_EQ(0, w);
}

TEST(Empty19, _calculatePattern_atari) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));

    Pat3 p;
    uint n,s,e,w;

    p = b._calculatePatternAt<3>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,1), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(0,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::BLACK());
    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(0, s);
    EXPECT_EQ(0, e);
    EXPECT_EQ(1, w);

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));

    //same block, but new atari info
    p = b._calculatePatternAt<3>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,1), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(0,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::BLACK());
    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);
}

TEST(Empty19, pattern_rotate) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));

    Pattern<5> p;
    uint n,s,e,w;

    p = b._calculatePatternAt<5>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);

    p = p.rotate();

    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::EMPTY());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WALL());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::WALL());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(1, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(0, w);

    EXPECT_NE(p, p.rotate());
    EXPECT_NE(p, p.rotate().rotate());
    EXPECT_NE(p, p.rotate().rotate().rotate());
    EXPECT_EQ(p, p.rotate().rotate().rotate().rotate());
}

TEST(Empty19, pattern_mirror_h) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));

    Pattern<5> p;
    uint n,s,e,w;

    p = b._calculatePatternAt<5>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::EMPTY());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(0, s);
    EXPECT_EQ(0, e);
    EXPECT_EQ(1, w);

    p = p.mirror_h();

    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(4,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(0,2), PointColor::EMPTY());

    EXPECT_EQ(p.getColorAt(4,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(0,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(4,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(0,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(0, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(0, w);

    EXPECT_NE(p, p.mirror_h());
    EXPECT_EQ(p, p.mirror_h().mirror_h());
}

TEST(Empty19, pattern_mirror_v) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));

    Pattern<5> p;
    uint n,s,e,w;

    p = b._calculatePatternAt<5>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);

    p = p.mirror_v();

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(1, n);
    EXPECT_EQ(0, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);

    EXPECT_NE(p, p.mirror_v());
    EXPECT_EQ(p, p.mirror_v().mirror_v());
}

TEST(Empty19, pattern_invert_colors) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,0));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 0,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 1,1));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 2,1));

    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 0,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 1,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,1));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,0));

    Pattern<5> p;
    uint n,s,e,w;

    p = b._calculatePatternAt<5>(COORD(1,0));
    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::WHITE());
    //EXPECT_EQ(p.getColorAt(2,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::WHITE());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);

    p = p.invert_colors();

    EXPECT_EQ(p.getColorAt(0,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,0), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,0), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(2,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(3,1), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(4,1), PointColor::WALL());

    EXPECT_EQ(p.getColorAt(0,2), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,2), PointColor::BLACK());
    //EXPECT_EQ(p.getColorAt(2,2), BoardStaTE::WALL());
    EXPECT_EQ(p.getColorAt(3,2), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,2), PointColor::BLACK());

    EXPECT_EQ(p.getColorAt(0,3), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,3), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(2,3), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(3,3), PointColor::WHITE());
    EXPECT_EQ(p.getColorAt(4,3), PointColor::BLACK());

    EXPECT_EQ(p.getColorAt(0,4), PointColor::WALL());
    EXPECT_EQ(p.getColorAt(1,4), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(2,4), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(3,4), PointColor::BLACK());
    EXPECT_EQ(p.getColorAt(4,4), PointColor::EMPTY());

    p.getAtaris(n,s,e,w);
    EXPECT_EQ(0, n);
    EXPECT_EQ(1, s);
    EXPECT_EQ(1, e);
    EXPECT_EQ(1, w);

    EXPECT_NE(p, p.invert_colors());
    EXPECT_EQ(p, p.invert_colors().invert_colors());
}

TEST(Empty19, pattern_canonical) {
    Board b(19);
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(0,0)),
        b.canonicalPatternAt<5>(PointColor::WHITE(), COORD(0,0))
    );

    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,15));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 15,3));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 15,15));

    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(4,2))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,4))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(4,4))
    );

    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,14))
    );

    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::WHITE(), COORD(14,14))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::WHITE(), COORD(14,16))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::WHITE(), COORD(16,14))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<5>(PointColor::BLACK(), COORD(2,2)),
        b.canonicalPatternAt<5>(PointColor::WHITE(), COORD(16,16))
    );

    //tengen, whole board
    EXPECT_EQ(
        b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(9,9)),
        b.canonicalPatternAt<19>(PointColor::WHITE(), COORD(9,9))
    );

    //big corners
    EXPECT_EQ(
        b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(0,0)),
        b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(0,18))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(0,0)),
        b.canonicalPatternAt<19>(PointColor::WHITE(), COORD(18,0))
    );
    EXPECT_EQ(
        b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(0,0)),
        b.canonicalPatternAt<19>(PointColor::WHITE(), COORD(18,18))
    );
}

TEST(Empty19, pattern_starKnight) {
    Board board_b1(19);
    board_b1.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    board_b1.playMoveAssumeLegal(Move(PointColor::WHITE(), 5,2));
    Pattern<13> pat_b1 = board_b1.canonicalPatternAt<13>(PointColor::BLACK(), COORD(3,3));

    Board board_b2(19);
    board_b2.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    board_b2.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,5));
    Pattern<13> pat_b2 = board_b2.canonicalPatternAt<13>(PointColor::BLACK(), COORD(3,3));

    Board board_w1(19);
    board_w1.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,3));
    board_w1.playMoveAssumeLegal(Move(PointColor::BLACK(), 5,2));
    Pattern<13> pat_w1 = board_w1.canonicalPatternAt<13>(PointColor::WHITE(), COORD(3,3));

    EXPECT_EQ(pat_b1, pat_b2);
    EXPECT_EQ(pat_b1, pat_w1);
}

TEST(Empty19, pattern_fromString) {
    std::string s = ":00001668";
    Pat3 p = Pat3::fromString(s);
    EXPECT_EQ(s, p.toString());
}

TEST(Empty19, pattern_fromString_fullGame) {
    std::string s = ":002aaa69:aaaaaaa4:015aaaa2:510066aa:8a550666:aaa69506:a2a0a514:522aa650:012aaaaa:615692aa:a146aa4a:a95016a4:2aa0111a:94aaa854:1aa4a881:08aa9aa8:5402aaaa:a04aaaa9:6a85208a:50aa0506:5912aa19:1a908aa1:90aaaaa0";
    Pattern<19> p = Pattern<19>::fromString(s);
    EXPECT_EQ(s, p.toString());
    p.dump();
}

TEST(Empty19, pattern_string_back_and_forth) {
    Board b(19);
    b.playMoveAssumeLegal(Move(PointColor::BLACK(), 3,3));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 2,3));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,2));
    b.playMoveAssumeLegal(Move(PointColor::WHITE(), 3,4));

    Pattern<19> p = b.canonicalPatternAt<19>(PointColor::BLACK(), COORD(4,3));
    std::string s = p.toString();

    Pattern<19> p2 = Pattern<19>::fromString(s);

    EXPECT_EQ(p, p2);
}


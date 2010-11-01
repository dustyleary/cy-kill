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

TEST_F(Empty19, size) {
    EXPECT_EQ(19, b.getSize());
}
TEST_F(Empty9, size) {
    EXPECT_EQ(9, b.getSize());
}

TEST_F(Empty19, countLiberties) {
    EXPECT_EQ(0, b.countLiberties(POS(0,0)));

    b.makeMoveAssumeLegal(BLACK, POS(0,0));
    EXPECT_EQ(2, b.countLiberties(POS(0,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,0));
    EXPECT_EQ(3, b.countLiberties(POS(0,0)));
    EXPECT_EQ(3, b.countLiberties(POS(1,0)));

    b.makeMoveAssumeLegal(BLACK, POS(1,1));
    b.dump();
    EXPECT_EQ(4, b.countLiberties(POS(0,0)));
    EXPECT_EQ(4, b.countLiberties(POS(1,0)));
    EXPECT_EQ(4, b.countLiberties(POS(1,1)));
}

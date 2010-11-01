#include "gtest/gtest.h"
#include "board.h"

class Empty19: public testing::Test {
public:
    Board b;
};

class Empty9: public testing::Test {
public:
    Empty9(): b(9) {}
    Board b;
};

TEST_F(Empty19, size) {
    EXPECT_EQ(19, b.getSize());
}
TEST_F(Empty9, size) {
    EXPECT_EQ(9, b.getSize());
}

TEST_F(Empty19, countLiberties) {
    EXPECT_EQ(0, b.countLiberties(POS(0,0)));
}

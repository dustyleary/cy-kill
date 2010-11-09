#include "gtest/gtest.h"
#include "config.h"

TEST(Gtp, preprocess_line) {
    EXPECT_EQ("", preprocess_line("   "));
    EXPECT_EQ("", preprocess_line(" \t  "));
    EXPECT_EQ("", preprocess_line(" \t#foo\t  "));
    EXPECT_EQ("19 do_something  2 2", preprocess_line("19 do_something  2 2 \t\n  "));
}


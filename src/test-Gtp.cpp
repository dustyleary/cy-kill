#include "gtest/gtest.h"
#include "config.h"

TEST(Gtp, preprocess_line) {
    EXPECT_EQ("", Gtp::preprocess_line("   "));
    EXPECT_EQ("", Gtp::preprocess_line(" \t  "));
    EXPECT_EQ("", Gtp::preprocess_line(" \t#foo\t  "));
    EXPECT_EQ("19 do_something  2 2", Gtp::preprocess_line("19 do_something  2 2 \t\n  "));
}

TEST(Gtp, parse_line) {
    GtpCommand gc;
    EXPECT_EQ(true, Gtp::parse_line("   ", gc));
    EXPECT_EQ("", gc.command);

    EXPECT_EQ(true, Gtp::parse_line(" 19 do something  ", gc));
    EXPECT_EQ(19, gc.id);
    EXPECT_EQ("do", gc.command);
    EXPECT_EQ(1, gc.args.size());
    EXPECT_EQ("something", gc.args[0]);

    EXPECT_EQ(true, Gtp::parse_line(" foo bar  baz#yuck", gc));
    EXPECT_EQ(-1, gc.id);
    EXPECT_EQ("foo", gc.command);
    EXPECT_EQ(2, gc.args.size());
    EXPECT_EQ("bar", gc.args[0]);
    EXPECT_EQ("baz", gc.args[1]);
}

TEST(Gtp, required_commands) {
    Gtp gtp;
    EXPECT_EQ("= 2\n\n", gtp.run_cmd("protocol_version"));
    EXPECT_EQ("= cy-kill\n\n", gtp.run_cmd("name"));
    EXPECT_EQ("= 0.1\n\n", gtp.run_cmd("version"));
}

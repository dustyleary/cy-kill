#include "gtest/gtest.h"
#include "config.h"

template<typename T>
struct EvensHelper {
    static T NatMap(T v) { return v>>1; }
};

class Simple: public testing::Test {
public:
    IntSet<int, 100> set;
};

class Evens: public testing::Test {
public:
    IntSet<short, 100, EvensHelper<short> > set;
};

TEST_F(Simple, simple) {
    EXPECT_EQ(0, set.size());
    set.add(0);
    EXPECT_EQ(1, set.size());
    set.add(10);
    EXPECT_EQ(2, set.size());
    set.add(43);
    set.add(45);
    set.add(44);
    set.add(46);
    EXPECT_EQ(6, set.size());
    EXPECT_EQ(true, set.contains(0));
    EXPECT_EQ(true, set.contains(10));
    EXPECT_EQ(true, set.contains(43));
    EXPECT_EQ(true, set.contains(45));
    EXPECT_EQ(true, set.contains(44));
    EXPECT_EQ(true, set.contains(46));
    EXPECT_EQ(false, set.contains(1));
}

TEST_F(Simple, remove) {
    EXPECT_EQ(0, set.size());
    set.add(1);
    EXPECT_EQ(1, set.size());
    set.remove(1);
    EXPECT_EQ(0, set.size());
    set.add(5);
    set.add(4);
    set.add(6);
    EXPECT_EQ(3, set.size());
    set.remove(4);
    EXPECT_EQ(2, set.size());
    set.remove(5);
    EXPECT_EQ(1, set.size());
    set.remove(6);
    EXPECT_EQ(0, set.size());
}

TEST_F(Simple, random) {
    for(int i=0; i<1e6; i++) {
        int v = rand() % 100;
        if(rand() & 1) {
            set.add(v);
            EXPECT_EQ(true, set.contains(v));
        } else {
            set.remove(v);
            EXPECT_EQ(false, set.contains(v));
        }
    }
    for(int i=0; i<100; i++) {
        set.remove(i);
    }
    EXPECT_EQ(0, set.size());
}

TEST_F(Evens, simple) {
    EXPECT_EQ(0, set.size());
    set.add(2);
    EXPECT_EQ(1, set.size());
    set.add(10);
    EXPECT_EQ(2, set.size());
    set.add(46);
    set.add(44);
    set.add(48);
    set.add(52);
    EXPECT_EQ(6, set.size());
}

TEST_F(Evens, remove) {
    EXPECT_EQ(0, set.size());
    set.add(0);
    EXPECT_EQ(1, set.size());
    set.remove(0);
    EXPECT_EQ(0, set.size());
    set.add(6);
    set.add(4);
    set.add(8);

    EXPECT_EQ(true, set.contains(6));
    EXPECT_EQ(true, set.contains(4));
    EXPECT_EQ(true, set.contains(8));

    EXPECT_EQ(3, set.size());
    set.remove(6);
    EXPECT_EQ(2, set.size());
    set.remove(4);
    EXPECT_EQ(1, set.size());
    set.remove(8);
    EXPECT_EQ(0, set.size());

    EXPECT_EQ(false, set.contains(6));
    EXPECT_EQ(false, set.contains(4));
    EXPECT_EQ(false, set.contains(8));
}

TEST_F(Evens, random) {
    for(int i=0; i<1e6; i++) {
        int v = 2 * (rand() % 50);
        if(rand() & 1) {
            set.add(v);
        } else {
            set.remove(v);
        }
    }
    for(int i=0; i<50; i++) {
        set.remove(i*2);
    }
    EXPECT_EQ(0, set.size());
}

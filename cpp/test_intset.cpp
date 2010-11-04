#include "gtest/gtest.h"
#include "intset.h"

template<typename T>
struct EvensHelper {
    static T MapToNat(T v) { return v>>1; }
    static T MapFromNat(T v) { return v<<1; }
};

class Simple: public testing::Test {
public:
    IntSet<int, 1000> set;
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
    set.add(63);
    set.add(64);
    set.add(65);
    set.add(76);
    EXPECT_EQ(10, set.size());
    EXPECT_EQ(true, set.contains(0));
    EXPECT_EQ(true, set.contains(10));
    EXPECT_EQ(true, set.contains(43));
    EXPECT_EQ(true, set.contains(45));
    EXPECT_EQ(true, set.contains(44));
    EXPECT_EQ(true, set.contains(46));
    EXPECT_EQ(true, set.contains(63));
    EXPECT_EQ(true, set.contains(64));
    EXPECT_EQ(true, set.contains(65));
    EXPECT_EQ(true, set.contains(76));
    EXPECT_EQ(false, set.contains(1));

    int bits[64];
    set.getValues(bits);
    EXPECT_EQ(bits[0], 0);
    EXPECT_EQ(bits[1], 10);
    EXPECT_EQ(bits[2], 43);
    EXPECT_EQ(bits[3], 44);
    EXPECT_EQ(bits[4], 45);
    EXPECT_EQ(bits[5], 46);
    EXPECT_EQ(bits[6], 63);
    EXPECT_EQ(bits[7], 64);
    EXPECT_EQ(bits[8], 65);
    EXPECT_EQ(bits[9], 76);
    EXPECT_EQ(bits[10], -1);
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
        int v = rand() % 1000;
        if(rand() & 1) {
            set.add(v);
            EXPECT_EQ(true, set.contains(v));
        } else {
            set.remove(v);
            EXPECT_EQ(false, set.contains(v));
        }
    }
    for(int i=0; i<1000; i++) {
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

    short bits[64];
    set.getValues(bits);
    EXPECT_EQ(bits[0], 2);
    EXPECT_EQ(bits[1], 10);
    EXPECT_EQ(bits[2], 44);
    EXPECT_EQ(bits[3], 46);
    EXPECT_EQ(bits[4], 48);
    EXPECT_EQ(bits[5], 52);
    EXPECT_EQ(bits[6], -1);
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

TEST(addAll, simple) {
    IntSet<int, 1000> set;
    for(int i=0; i<1000; i++) {
        set.add(i);
    }
    IntSet<int, 1000> set2;
    set2.addAll(set);
    for(int i=0; i<1000; i++) {
        EXPECT_EQ(true, set2.contains(i));
    }
}

TEST(count_bits_set, simple) {
    EXPECT_EQ(0, count_bits_set(0));
    EXPECT_EQ(1, count_bits_set(1));
    EXPECT_EQ(1, count_bits_set(2));
    EXPECT_EQ(1, count_bits_set(4));
    EXPECT_EQ(1, count_bits_set(8));

    EXPECT_EQ(2, count_bits_set(3));
    EXPECT_EQ(64, count_bits_set(0xffffffffffffffffLL));
}


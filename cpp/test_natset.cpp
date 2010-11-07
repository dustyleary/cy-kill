#include "gtest/gtest.h"
#include "config.h"

struct Nat999 : public Nat<Nat999> {
    PRIVATE_NAT_CONSTRUCTOR(Nat999);

    static const uint kBound = 999;
};

TEST(NatSet, add_remove) {
    NatSet<Nat999> ns;
    ForEachNat(Nat999, i) {
        EXPECT_EQ(false, ns.contains(i));
        ns.add(i);
        EXPECT_EQ(true, ns.contains(i));
        ns.remove(i);
        EXPECT_EQ(false, ns.contains(i));
    }
}

TEST(NatSet, clear) {
    NatSet<Nat999> ns;
    ForEachNat(Nat999, i) {
        ns.add(i);
    }
    ForEachNat(Nat999, i) {
        EXPECT_EQ(true, ns.contains(i));
    }
    ns.clear();
    ForEachNat(Nat999, i) {
        EXPECT_EQ(false, ns.contains(i));
    }
}


#include "gtest/gtest.h"
#include "config.h"

struct Nat999 : public Nat<Nat999> {
    PRIVATE_NAT_CONSTRUCTOR(Nat999);

    static const uint kBound = 999;
};

TEST(NatSet, add_remove) {
    NatSet<Nat999> ns;
    uint n = 0;
    FOREACH_NAT(Nat999, i, {
        n++;
        EXPECT_EQ(false, ns.contains(i));
        ns.add(i);
        EXPECT_EQ(true, ns.contains(i));
        ns.remove(i);
        EXPECT_EQ(false, ns.contains(i));
    });
    EXPECT_EQ(999, n);
}

TEST(NatSet, add_remove_funky_order) {
    NatSet<Nat999> ns;
    FOREACH_NAT(Nat999, i, {
        EXPECT_EQ(false, ns.contains(i));
        ns.add(i);
        EXPECT_EQ(true, ns.contains(i));
    });
    FOREACH_NAT(Nat999, i, {
        if(i.toUint() % 3) {
            continue;
        }
        EXPECT_EQ(true, ns.contains(i));
        ns.remove(i);
        EXPECT_EQ(false, ns.contains(i));
    });
    FOREACH_NAT(Nat999, i, {
        if(i.toUint() % 3) {
            EXPECT_EQ(true, ns.contains(i));
        } else {
            EXPECT_EQ(false, ns.contains(i));
        }
    });
}

TEST(NatSet, reset) {
    NatSet<Nat999> ns;
    FOREACH_NAT(Nat999, i, {
        ns.add(i);
    });
    FOREACH_NAT(Nat999, i, {
        EXPECT_EQ(true, ns.contains(i));
    });
    ns.reset();
    FOREACH_NAT(Nat999, i, {
        EXPECT_EQ(false, ns.contains(i));
    });
}


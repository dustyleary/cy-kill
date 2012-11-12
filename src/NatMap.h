#pragma once

//typesafe array indexed my Nat
template<typename Nat, typename E>
struct NatMap {
    E a[Nat::kBound];

    explicit NatMap() {}
    explicit NatMap(E v) { setAll(v); }
    void setAll(E v) {
        FOREACH_NAT(Nat, i, { a[i.toUint()] = v; });
    }

    E& operator[](Nat n) { return a[n.toUint()]; }
    const E& operator[](Nat n) const { return a[n.toUint()]; }
};


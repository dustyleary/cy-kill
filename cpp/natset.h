#pragma once

#include "config.h"
#include "nat.h"
#include "natmap.h"

template<typename Nat>
struct NatSet {
    explicit NatSet() : marks(0), active_mark(1) {}

    void clear() { active_mark++; }
    bool contains(Nat n) { return marks[n] == active_mark; }
    void add(Nat n) { marks[n] = active_mark; }
    void remove(Nat n) { marks[n] = active_mark-1; }

    NatMap<Nat, uint> marks;
    uint active_mark;
};


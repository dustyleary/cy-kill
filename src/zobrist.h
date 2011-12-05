#pragma once

struct Zobrist {
    static NatMap<Point, uint64_t> black;
    static NatMap<Point, uint64_t> white;

    static void init() {
        LOG("Zobrist::init()");
        FOREACH_NAT(Point, p, {
            black[p] = gen_rand64();
            white[p] = gen_rand64();
        });
    }
};


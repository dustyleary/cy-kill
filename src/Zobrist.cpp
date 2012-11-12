#include "config.h"

GlobalInitializer gGlobalInitializer;

NatMap<Point, uint64_t> Zobrist::black;
NatMap<Point, uint64_t> Zobrist::white;


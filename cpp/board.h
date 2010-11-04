#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <stdint.h>

#include <algorithm>

#include "intset.h"

//board data structure pretty much described in
//http://www.eecs.harvard.edu/econcs/pubs/Harrisonthesis.pdf

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int WALL = 3;
static const char stateChars[] = ".XO#";

#define ENEMY(c) (c^3)

#define POS(x, y) (((x+1)<<8) | (y+1))
#define X(p) ((p>>8)-1)
#define Y(p) ((p&0xff)-1)

#define N(p) (p-1)
#define S(p) (p+1)
#define E(p) (p+(1<<8))
#define W(p) (p-(1<<8))

typedef uint8_t BoardState;
typedef uint16_t Point;
typedef uint8_t ChainIndex;

#undef assert
#define STR2(V) #V
#define STR(V) STR2(V)
#define ASSERT(expr) \
    if(!(expr)) throw std::runtime_error("ASSERT FAILED (" __FILE__ ":" STR(__LINE__) "): " #expr);

template<int SIZE>
struct Board {
    static const int BOARD_ARRAY_SIZE = (SIZE+2)*(SIZE+1)+1;
    static const int PLAY_SIZE = SIZE*SIZE;
    static const int MAX_CHAINS = (PLAY_SIZE*3)/4;

    static uint8_t getSize() { return SIZE; }
    static int offset(Point p) { return (X(p)+1)+(Y(p)+1)*(SIZE+1); }

    static Point MapToNat(Point p) { return X(p)+Y(p)*SIZE; }
    static Point MapFromNat(Point p) { Point x = p%SIZE; Point y=p/SIZE; return POS(x,y); }

    typedef IntSet<Point, PLAY_SIZE, Board> PointSet;

    struct Chain {
        PointSet stones;
        PointSet liberties;
        bool dead;
        void reset() {
            stones.reset();
            liberties.reset();
            dead = false;
        }
    };
    Chain chains[MAX_CHAINS];

    BoardState states[BOARD_ARRAY_SIZE];
    ChainIndex chain_indexes[BOARD_ARRAY_SIZE];
    int chain_count;
    Point koPoint;
    PointSet emptyPoints;
    Point setValues[PLAY_SIZE*2];

    Board() {
        reset();
    }

    void reset() {
        chain_count = 0;
        koPoint = POS(-1,-1);
        memset(states, EMPTY, sizeof(states));
        memset(chain_indexes, 0, sizeof(chain_indexes));
        for(int y=0; y<(SIZE+2); y++) {
            states[y*(SIZE+1)] = WALL;
        }
        for(int x=0; x<(SIZE+2); x++) {
            states[x] = WALL;
            states[(SIZE+1)*(SIZE+1) + x] = WALL;
        }
        emptyPoints.reset();
        for(int x=0; x<SIZE; x++) {
            for(int y=0; y<SIZE; y++) {
                emptyPoints.add(POS(x,y));
            }
        }
    }

    void assertGoodState() {
        //walls are intact
        for(int y=0; y<(SIZE+2); y++) {
            ASSERT(states[y*(SIZE+1)] == WALL);
        }
        for(int x=0; x<(SIZE+2); x++) {
            ASSERT(states[x] == WALL);
            ASSERT(states[(SIZE+1)*(SIZE+1) + x] == WALL);
        }
        //empty points are correct
        for(int y=0; y<SIZE; y++) {
            for(int x=0; x<SIZE; x++) {
                Point p = POS(x,y);
                if(bs(p) == EMPTY) {
                    ASSERT(emptyPoints.contains(p));
                } else {
                    ASSERT(!emptyPoints.contains(p));
                }
            }
        }
    }

    void dump() {
        for(int y=0; y<(SIZE+2); y++) {
            for(int x=0; x<(SIZE+2); x++) {
                putc(stateChars[states[y*(SIZE+1)+x]], stdout);
            }
            putc('\n', stdout);
        }
		fflush(stdout);
    }

    BoardState& bs(Point p) { return states[offset(p)]; }

    uint8_t countLiberties(Point p) {
        BoardState v = bs(p);
        if(v == EMPTY) return 0;
        int ci = chain_indexes[offset(p)]-1;
        Chain& c = chains[ci];
        return c.liberties.size();
    }

    void mergeChains(Point dest, Point inc) {
        int di = chain_indexes[offset(dest)]-1;
        int ii = chain_indexes[offset(inc)]-1;
        if(di == ii) return;
        Chain& cd = chains[di];
        Chain& ci = chains[ii];
        cd.liberties.addAll(ci.liberties);
        cd.stones.addAll(ci.stones);
        cd.stones.getValues(setValues);
        Point* p = setValues;
        while(*p != Point(-1)) {
            chain_indexes[offset(*p++)] = di+1;
        }
    }

    void chainAddPoint(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.stones.add(p);

        chain_indexes[offset(p)] = ci+1;

#define doit(D) \
        if(bs(D(p)) == EMPTY) { c.liberties.add(D(p)); } \
        else if(bs(D(p)) == bs(p)) { mergeChains(p, D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        c.liberties.remove(p);
    }

    void chainAddLiberty(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.liberties.add(p);
    }

    void makeNewChain(Point p) {
        int ci;
        for(ci=0; ci<chain_count; ci++) {
            //try to find a chain index to reuse
            if(chains[ci].dead) {
                break;
            }
        }
        if(ci == chain_count) {
            chain_count++;
        }
        chain_indexes[offset(p)] = ci+1;

        Chain& c = chains[ci];
        c.reset();
        c.stones.add(p);
#define doit(D) if(bs(D(p)) == EMPTY) { c.liberties.add(D(p)); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
    }

    void chainRemoveLiberty(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        Chain& c = chains[ci];
        c.liberties.remove(p);
        if(c.liberties.size() == 0) {
            //kill
            c.stones.getValues(setValues);
            Point* p = setValues;
            while(*p != Point(-1)) {
                bs(*p) = EMPTY;
                emptyPoints.add(*p);
                chain_indexes[offset(*p)] = 0;
                p++;
            }

            p = setValues;
            while(*p != Point(-1)) {
#define doit(D) if(bs(D(*p)) == BLACK || bs(D(*p)) == WHITE) { chainAddLiberty(D(*p), *p); }
                doit(N)
                doit(S)
                doit(E)
                doit(W)
#undef doit
                p++;
            }
            if(c.stones.size() == 1) {
                koPoint = setValues[0];
            }
            c.dead = true;
        }
    }

    void makeMoveAssumeLegal(BoardState c, Point p) {
        koPoint = POS(-1, -1);

        bs(p) = c;
        if(false) {}
#define doit(D) else if(bs(D(p)) == c) { chainAddPoint(D(p), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        else {
            makeNewChain(p);
        }

        BoardState ec = ENEMY(c);
#define doit(D) if(bs(D(p)) == ec) { chainRemoveLiberty(D(p), p); }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        emptyPoints.remove(p);
    }

    bool isSuicide(BoardState c, Point p) {
#define doit(D) if(bs(D(p)) == EMPTY) { return false; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        BoardState ec = ENEMY(c);
#define doit(D) if(bs(D(p)) == ec && (countLiberties(D(p)) == 1)) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

#define doit(D) if(bs(D(p)) == c && countLiberties(D(p)) > 1) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit

        return true;
    }

    bool isSimpleEye(BoardState c, Point p) {
        if(bs(p) != EMPTY) return false;
#define doit(D) if(bs(D(p)) != c && bs(D(p)) != WALL) return false;
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        BoardState ec = ENEMY(c);
        int enemy_diagonal_count = 0;
#define doit(D,F) if(bs(D(F(p))) == ec) { enemy_diagonal_count++; }
        doit(N,E)
        doit(N,W)
        doit(S,E)
        doit(S,W)
#undef doit
        if(enemy_diagonal_count == 0) return true;
        if(enemy_diagonal_count >= 2) return false;

        int wall_count = 0;
#define doit(D) if(bs(D(p)) == WALL) { wall_count++; }
        doit(N)
        doit(S)
        doit(E)
        doit(W)
#undef doit
        if(wall_count>1) return false;
        return true;
    }

    bool hasKoPoint() {
        return koPoint != POS(-1,-1);
    }

    void mcgMoves(BoardState c, PointSet& ps) {
        memcpy(&ps, &emptyPoints, sizeof(ps));
        if(koPoint != POS(-1,-1)) {
            ps.remove(koPoint);
        }
        for(int i=0; i<emptyPoints._size; i++) {
            emptyPoints.getValues(setValues);
            Point* p = setValues;
            while(*p != Point(-1)) {
                if(isSimpleEye(c,*p) || isSuicide(c,*p)) { ps.remove(*p); }
                p++;
            }
        }
    }
};


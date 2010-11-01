#pragma once

#include <stdint.h>
#include <stdio.h>
#include <exception>

//board data structure pretty much described in
//http://www.eecs.harvard.edu/econcs/pubs/Harrisonthesis.pdf

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int WALL = 3;
static const char stateChars[] = ".XO#";

#define POS(x, y) (((x+1)<<8) | (y+1))
#define X(p) ((p>>8))
#define Y(p) ((p&0xff))

#define N(p) (p-1)
#define S(p) (p+1)
#define E(p) (p+(1<<8))
#define W(p) (p-(1<<8))

typedef int BoardState;
typedef int Point;
typedef int PointSetIndex;
typedef int ChainIndex;

template<int SIZE>
struct Board {
    static const int BOARD_ARRAY_SIZE = (SIZE+2)*(SIZE+1)+1;
    static const int PLAY_SIZE = SIZE*SIZE;
    static const int MAX_CHAINS = PLAY_SIZE/2;

    static uint8_t getSize() { return SIZE; }
    static int offset(Point p) { return X(p)+Y(p)*(SIZE+1); }

    struct PointSet {
        void reset() {
            _size = 0;
            memset(_point_indexes, 0, sizeof(_point_indexes));
        }

        void add(Point p) {
            PointSetIndex i = _point_indexes[offset(p)]-1;
            if(i!=-1) return;
            i = _size++;
            _list_of_points[i] = p;
            _point_indexes[offset(p)] = i+1;
        }
        void remove(Point p) {
            PointSetIndex i = _point_indexes[offset(p)]-1;
            if(i==-1) return;
            std::swap(_list_of_points[i], _list_of_points[_size-1]);
            _size--;
            _point_indexes[offset(p)] = 0;
        }

        int size() { return _size; }

        void dump() {
            printf("{");
            for(int i=0; i<_size; i++) {
                Point p = _list_of_points[i];
                printf("(%d,%d)", X(p)-1, Y(p)-1);
                if(i<(_size-1)) {
                    printf(", ");
                }
            }
            printf("}");
        }
        int _size;
        int _point_indexes[BOARD_ARRAY_SIZE];
        Point _list_of_points[PLAY_SIZE];
    };
    struct Chain {
        PointSet stones;
        PointSet liberties;
        void reset() {
            stones.reset();
            liberties.reset();
        }
    };
    Chain chains[MAX_CHAINS];

    BoardState states[BOARD_ARRAY_SIZE];
    ChainIndex chain_indexes[BOARD_ARRAY_SIZE];
    int chain_count;

    Board() : chain_count(0) {
        memset(states, EMPTY, sizeof(states));
        memset(chain_indexes, 0, sizeof(chain_indexes));
        for(int y=0; y<(SIZE+2); y++) {
            states[y*(SIZE+1)] = WALL;
        }
        for(int x=0; x<(SIZE+2); x++) {
            states[x] = WALL;
            states[(SIZE+1)*(SIZE+1) + x] = WALL;
        }
    }

    void dump() {
        for(int y=0; y<(SIZE+2); y++) {
            for(int x=0; x<(SIZE+2); x++) {
                putc(stateChars[states[y*(SIZE+1)+x]], stdout);
            }
            putc('\n', stdout);
        }
    }

    BoardState& bs(Point p) { return states[offset(p)]; }

    uint8_t countLiberties(Point p) {
        BoardState v = bs(p);
        if(v == EMPTY) return 0;
        int ci = chain_indexes[offset(p)]-1;
        Chain& c = chains[ci];
        return c.liberties.size();
    }

    void chainAddPoint(Point chain, Point p) {
        int ci = chain_indexes[offset(chain)]-1;
        chain_indexes[offset(p)] = ci+1;
        Chain& c = chains[ci];
        c.stones.add(p);
        c.liberties.remove(p);
        if(bs(N(p)) == EMPTY) { c.liberties.add(N(p)); }
        if(bs(S(p)) == EMPTY) { c.liberties.add(S(p)); }
        if(bs(E(p)) == EMPTY) { c.liberties.add(E(p)); }
        if(bs(W(p)) == EMPTY) { c.liberties.add(W(p)); }
    }

    void makeNewChain(Point p) {
        int ci = chain_count++;
        chain_indexes[offset(p)] = ci+1;

        Chain& c = chains[ci];
        c.reset();
        c.stones.add(p);
        if(bs(N(p)) == EMPTY) { c.liberties.add(N(p)); }
        if(bs(S(p)) == EMPTY) { c.liberties.add(S(p)); }
        if(bs(E(p)) == EMPTY) { c.liberties.add(E(p)); }
        if(bs(W(p)) == EMPTY) { c.liberties.add(W(p)); }
    }

    void makeMoveAssumeLegal(BoardState color, Point p) {
        bs(p) = color;
        if(false) {}
        else if(bs(N(p)) == color) { chainAddPoint(N(p), p); }
        else if(bs(E(p)) == color) { chainAddPoint(E(p), p); }
        else if(bs(S(p)) == color) { chainAddPoint(S(p), p); }
        else if(bs(W(p)) == color) { chainAddPoint(W(p), p); }
        else { makeNewChain(p); }
    }
};


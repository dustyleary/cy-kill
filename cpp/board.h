#pragma once

#include <stdint.h>
#include <stdio.h>
#include <exception>

const int EMPTY = 0;
const int BLACK = 1;
const int WHITE = 2;
const int WALL = 3;
static const char stateChars[] = ".XO#";

#define MAX_BOARD_SIZE 19
#define BOARD_ARRAY_SIZE ((MAX_BOARD_SIZE+2)*(MAX_BOARD_SIZE+1)+1)
#define POS(x, y) (((x+1)<<8) | (y+1))
#define X(p) ((p>>8))
#define Y(p) ((p&0xff))

typedef int BoardState;
typedef int Point;

struct Board {
    BoardState states[BOARD_ARRAY_SIZE];
    Point next_point_in_chain[BOARD_ARRAY_SIZE];

    Board(uint8_t s=19) : size(s) {
        memset(states, EMPTY, sizeof(states));
        for(int y=0; y<(size+2); y++) {
            states[y*(size+1)] = WALL;
        }
        for(int x=0; x<(size+2); x++) {
            states[x] = WALL;
            states[(size+1)*(size+1) + x] = WALL;
        }
    }

    void dump() {
        for(int y=0; y<(size+2); y++) {
            for(int x=0; x<(size+2); x++) {
                putc(stateChars[states[y*(size+1)+x]], stdout);
            }
            putc('\n', stdout);
        }
    }

    uint8_t size;
    uint8_t getSize() const { return size; }
    BoardState& bs(Point p) {
        return states[X(p)+Y(p)*(size+1)];
    }
    uint8_t countLiberties(Point p) {
        BoardState v = bs(p);
        if(v == EMPTY) return 0;
        throw std::runtime_error("unhandled BoardState");
    }
};


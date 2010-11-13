#pragma once

struct ChainInfo {
    uint liberty_count;
    uint liberty_sum;
    uint liberty_sum_squares;
    uint _size;

    ChainInfo() { reset(); }
    void reset() {
        liberty_count = 0;
        liberty_sum = 0;
        liberty_sum_squares = 0;
        _size = 0;
    }
    uint size() const { return _size; }
    bool isDead() const { return liberty_count == 0; }

    void addStone(Point p) { _size++; }
    void addLiberty(Point p) {
        liberty_count++;
        liberty_sum += p.toUint();
        liberty_sum_squares += p.toUint()*p.toUint();
    }
    void removeLiberty(Point p) {
        liberty_count--;
        liberty_sum -= p.toUint();
        liberty_sum_squares -= p.toUint()*p.toUint();
    }
    void merge(ChainInfo& o) {
        _size += o._size;
        liberty_count += o.liberty_count;
        liberty_sum += o.liberty_sum;
        liberty_sum_squares += o.liberty_sum_squares;
    }
    bool isInAtari() const {
        return (liberty_count * liberty_sum_squares) == (liberty_sum * liberty_sum);
    }
};

#define FOREACH_CHAIN_STONE(chainPt, pt, block) \
    { \
        Point orig = chainPt; \
        Point pt = chainPt; \
        do { \
            block \
            pt = chain_next_point[pt]; \
        } while(pt != orig); \
    }


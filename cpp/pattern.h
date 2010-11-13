#pragma once

template<uint N>
struct Pattern {
    static const uint kNumPoints = N*N - 1;
    static const uint kNumBits = kNumPoints*2 + 4;
    static const uint kNumUints = (kNumBits + 31) / 32;
    static const uint kBound = 2<<kNumBits;

    uint32_t data[kNumUints];
    Pattern() {
        memset(data, 0, sizeof(data));
    }

    uint getPointId(Point p) const {
        ASSERT(p.x() < N);
        ASSERT(p.y() < N);
        ASSERT(!(p.x() == (N/2) && p.y() == (N/2)));
        uint point_id = p.y() * N + p.x();
        if(p.y() > (N/2) || (p.y()==(N/2) && p.x()>(N/2))) {
            point_id--;
        }
        return point_id;
    }

    BoardState getColorAt(Point p) const {
        uint shift = 4 + getPointId(p)*2;
        uint bits = (data[0] >> shift) & 3;
        return BoardState::fromUint(bits);
    }

    void setColorAt(Point p, BoardState c) {
        uint shift = 4 + getPointId(p)*2;
        data[0] &= ~(3<<shift);
        data[0] |= (c.toUint()<<shift);
    }

    void setAtaris(uint N, uint S, uint E, uint W) {
        data[0] &= ~15;
        data[0] |= (N<<3) | (S<<2) | (E<<1) | W;
    }

    void getAtaris(uint& N, uint& S, uint& E, uint& W) {
        N = (data[0]>>3) & 1;
        S = (data[0]>>2) & 1;
        E = (data[0]>>1) & 1;
        W = (data[0]>>0) & 1;
    }

    bool operator==(const Pattern& o) const {
        for(uint i=0; i<kNumUints; i++) {
            if(data[i] != o.data[i]) return false;
        }
        return true;
    }
    bool operator!=(const Pattern& o) const {
        return !(operator==(o));
    }

    std::string toString() const {
        std::string result = "";
        for(uint i=0; i<kNumUints; i++) {
            result = strprintf(":%08x", data[i]) + result;
        }
        return result;
    }
};

typedef Pattern<3> Pat3;


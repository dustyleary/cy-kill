#pragma once

template<uint N>
struct Pattern {
    static const uint kNumPoints = N*N - 1;
    static const uint kNumBits = kNumPoints*2 + 4;
    static const uint kNumUints = (kNumBits + 31) / 32;

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
        uint byte = shift/32;
        uint shiftbits = shift%32;
        uint bits = (data[byte] >> shiftbits) & 3;
        return BoardState::fromUint(bits);
    }

    void setColorAt(Point p, BoardState c) {
        uint shift = 4 + getPointId(p)*2;
        uint byte = shift/32;
        uint shiftbits = shift%32;
        data[byte] &= ~(3<<shiftbits);
        data[byte] |= (c.toUint()<<shiftbits);
        ASSERT(getColorAt(p) == c);
    }

    void setAtaris(uint n, uint s, uint e, uint w) {
        data[0] &= ~15;
        data[0] |= (n<<3) | (s<<2) | (e<<1) | w;
    }

    void getAtaris(uint& n, uint& s, uint& e, uint& w) const {
        n = (data[0]>>3) & 1;
        s = (data[0]>>2) & 1;
        e = (data[0]>>1) & 1;
        w = (data[0]>>0) & 1;
    }

    void dump() {
        for(int y=0; y<N; y++) {
            for(int x=0; x<N; x++) {
                putc(getColorAt(COORD(x,y)).stateChar(), stdout);
            }
            putc('\n', stdout);
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        putc(n?'1':'0', stdout);
        putc(s?'1':'0', stdout);
        putc(e?'1':'0', stdout);
        putc(w?'1':'0', stdout);
        putc('\n', stdout);
        fflush(stdout);
    }

    Pattern rotate() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == (N/2) && x == (N/2)) {
                    continue;
                }
                BoardState c = getColorAt(COORD(x,y));
                r.setColorAt(COORD(y,N-1-x), c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(e,w,s,n);
        return r;
    }

    Pattern mirror_h() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == (N/2) && x == (N/2)) {
                    continue;
                }
                BoardState c = getColorAt(COORD(x,y));
                r.setColorAt(COORD(N-1-x,y), c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(n,s,w,e);
        return r;
    }

    Pattern mirror_v() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == (N/2) && x == (N/2)) {
                    continue;
                }
                BoardState c = getColorAt(COORD(x,y));
                r.setColorAt(COORD(x,N-1-y), c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(s,n,e,w);
        return r;
    }

    Pattern invert_colors() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == (N/2) && x == (N/2)) {
                    continue;
                }
                BoardState c = getColorAt(COORD(x,y));
                if(c.isPlayer()) {
                    c = c.enemy();
                }
                r.setColorAt(COORD(x,y), c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(n,s,e,w);
        return r;
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
    bool operator<(const Pattern& o) const {
        for(int i=(int)kNumUints-1; i>=0; i--) {
            if(data[i] < o.data[i]) return true;
            if(data[i] > o.data[i]) return false;
        }
        return false;
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
typedef Pattern<5> Pat5;
typedef Pattern<7> Pat7;
typedef Pattern<9> Pat9;
typedef Pattern<11> Pat11;


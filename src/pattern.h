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
    Pattern(const Pattern& rhs) {
        *this = rhs;
    }

    Pattern& operator=(const Pattern& rhs) {
        memcpy(data, rhs.data, sizeof(data));
        return *this;
    }

    static uint mid() {
        return (N-1)/2;
    }

    bool isMidPoint(int x, int y) const {
        return x == mid() && y == mid();
    }

    uint getPointId(int x, int y) const {
        ASSERT(x < N);
        ASSERT(y < N);
        ASSERT(!isMidPoint(x,y));
        uint point_id = y*N + x;
        if(point_id >= (mid()*N+mid())) {
            point_id--;
        }
        return point_id;
    }

    BoardState getColorAt(int x, int y) const {
        uint shift = 4 + getPointId(x,y)*2;
        uint byte = shift/32;
        uint shiftbits = shift%32;
        uint bits = (data[byte] >> shiftbits) & 3;
        return BoardState::fromUint(bits);
    }

    void setColorAt(int x, int y, BoardState c) {
        uint shift = 4 + getPointId(x,y)*2;
        uint byte = shift/32;
        uint shiftbits = shift%32;
        data[byte] &= ~(3<<shiftbits);
        data[byte] |= (c.toUint()<<shiftbits);
        ASSERT(getColorAt(x, y) == c);
    }

    bool isSuicide() const {
      BoardState nc = getColorAt(1,0);
      BoardState sc = getColorAt(1,2);
      BoardState ec = getColorAt(2,1);
      BoardState wc = getColorAt(0,1);

      //empty neighbor
      if(nc == BoardState::EMPTY()) return false;
      if(sc == BoardState::EMPTY()) return false;
      if(ec == BoardState::EMPTY()) return false;
      if(wc == BoardState::EMPTY()) return false;

      uint n,s,e,w;
      getAtaris(n,s,e,w);
      //atari enemy
      if(n && (nc == BoardState::WHITE())) return false;
      if(s && (sc == BoardState::WHITE())) return false;
      if(e && (ec == BoardState::WHITE())) return false;
      if(w && (wc == BoardState::WHITE())) return false;

      //non-atari friend
      if((!n) && (nc == BoardState::BLACK())) return false;
      if((!s) && (sc == BoardState::BLACK())) return false;
      if((!e) && (ec == BoardState::BLACK())) return false;
      if((!w) && (wc == BoardState::BLACK())) return false;

      return true;
    }

    bool isSimpleEye() const {
      BoardState nc = getColorAt(1,0);
      BoardState sc = getColorAt(1,2);
      BoardState ec = getColorAt(2,1);
      BoardState wc = getColorAt(0,1);
      
      //direct neighbor is empty or enemy
      if((nc == BoardState::EMPTY()) || (nc == BoardState::WHITE())) return false;
      if((sc == BoardState::EMPTY()) || (sc == BoardState::WHITE())) return false;
      if((ec == BoardState::EMPTY()) || (ec == BoardState::WHITE())) return false;
      if((wc == BoardState::EMPTY()) || (wc == BoardState::WHITE())) return false;

      NatMap<BoardState, uint> diagonal_counts(0);
      diagonal_counts[getColorAt(0,0)]++;
      diagonal_counts[getColorAt(2,0)]++;
      diagonal_counts[getColorAt(0,2)]++;
      diagonal_counts[getColorAt(2,2)]++;

      return (diagonal_counts[BoardState::WHITE()] + (diagonal_counts[BoardState::WALL()]>0)) < 2;
    }

    void resetAtaris() {
        data[0] &= ~15;
    }
    void setAtaris(uint n, uint s, uint e, uint w) {
        data[0] |= (n<<3) | (s<<2) | (e<<1) | w;
    }
    void clearAtaris(uint n, uint s, uint e, uint w) {
        data[0] &= ~((n<<3) | (s<<2) | (e<<1) | w);
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
                char c = isMidPoint(x,y) ? '.' : getColorAt(x,y).stateChar();
                putc(c, stderr);
            }
            putc('\n', stderr);
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        putc(n?'1':'0', stderr);
        putc(s?'1':'0', stderr);
        putc(e?'1':'0', stderr);
        putc(w?'1':'0', stderr);
        putc('\n', stderr);
        fflush(stderr);
    }

    Pattern rotate() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == mid() && x == mid()) {
                    continue;
                }
                BoardState c = getColorAt(x,y);
                r.setColorAt(y,N-1-x, c);
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
                if(y == mid() && x == mid()) {
                    continue;
                }
                BoardState c = getColorAt(x,y);
                r.setColorAt(N-1-x,y, c);
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
                if(y == mid() && x == mid()) {
                    continue;
                }
                BoardState c = getColorAt(x,y);
                r.setColorAt(x,N-1-y, c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(s,n,e,w);
        return r;
    }

    Pattern _calculate_inverted_colors() const {
        Pattern r;
        for(uint y=0; y<N; y++) {
            for(uint x=0; x<N; x++) {
                if(y == mid() && x == mid()) {
                    continue;
                }
                BoardState c = getColorAt(x,y);
                if(c.isPlayer()) {
                    c = c.enemy();
                }
                r.setColorAt(x,y, c);
            }
        }
        uint n,s,e,w;
        getAtaris(n,s,e,w);
        r.setAtaris(n,s,e,w);
        return r;
    }
    Pattern invert_colors() const;

    Pattern canonical() const {
        Pattern<N> p = *this;
        Pattern<N> r = p;
        for(uint i=0; i<5; i++) {
            p = p.rotate();
            Pattern<N> p2 = p;
            if(p<r) r=p;
            p2 = p2.mirror_h();
            if(p<r) r=p;
            p2 = p2.mirror_v();
            if(p<r) r=p;
            p2 = p2.mirror_h();
            if(p<r) r=p;
        }
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

    uint32_t toUint() const {
        return data[0];
    }

    static Pattern fromUint(uint32_t v) {
        Pattern result;
        result.data[0] = v;
        return result;
    }

    std::string toString() const {
        std::string result = "";
        for(uint i=0; i<kNumUints; i++) {
            result = strprintf(":%08x", data[i]) + result;
        }
        return result;
    }

    static Pattern fromString(const std::string& s) {
        Pattern p;
        for(uint i=0; i<kNumUints; i++) {
            std::string intstr = s.substr(i*9+1, 8);
            p.data[kNumUints-1-i] = (uint32_t) strtol(intstr.c_str(), NULL, 16);
        }
        ASSERT(p.toString() == s);
        return p;
    }
};

typedef Pattern<3> Pat3;
typedef Pattern<5> Pat5;
typedef Pattern<7> Pat7;
typedef Pattern<9> Pat9;
typedef Pattern<11> Pat11;

template <uint N>
Pattern<N> Pattern<N>::invert_colors() const {
    return _calculate_inverted_colors();
}

extern Pat3 getPat3InvertedColors(uint patternId);
template<>
inline Pattern<3> Pattern<3>::invert_colors() const {
    return getPat3InvertedColors(toUint());
}


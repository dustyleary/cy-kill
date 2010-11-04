#pragma once

#include <stdint.h>

template<typename T>
struct SimpleHelper {
    static T MapToNat(T v) { return v; }
    static T MapFromNat(T v) { return v; }
};

//http://www-graphics.stanford.edu/~seander/bithacks.html

//The following 64-bit code selects the position of the rth 1 bit when counting from the left. In other words if we start at the most significant bit and proceed to the right, counting the number of bits set to 1 until we reach the desired rank, r, then the position where we stop is returned. If the rank requested exceeds the count of bits set, then 64 is returned. The code may be modified for 32-bit or counting from the right.
unsigned int nth_bit_position(uint64_t v, unsigned int r) {
  unsigned int s;      // Output: Resulting position of bit with rank r [1-64]
  uint64_t a, b, c, d; // Intermediate temporaries for bit count.
  unsigned int t;      // Bit count temporary.

  // Do a normal parallel bit count for a 64-bit integer,                     
  // but store all intermediate steps.                                        
  // a = (v & 0x5555...) + ((v >> 1) & 0x5555...);
  a =  v - ((v >> 1) & ~0UL/3);
  // b = (a & 0x3333...) + ((a >> 2) & 0x3333...);
  b = (a & ~0UL/5) + ((a >> 2) & ~0UL/5);
  // c = (b & 0x0f0f...) + ((b >> 4) & 0x0f0f...);
  c = (b + (b >> 4)) & ~0UL/0x11;
  // d = (c & 0x00ff...) + ((c >> 8) & 0x00ff...);
  d = (c + (c >> 8)) & ~0UL/0x101;
  t = (d >> 32) + (d >> 48);
  // Now do branchless select!                                                
  s  = 64;
  // if (r > t) {s -= 32; r -= t;}
  s -= ((t - r) & 256) >> 3; r -= (t & ((t - r) >> 8));
  t  = (d >> (s - 16)) & 0xff;
  // if (r > t) {s -= 16; r -= t;}
  s -= ((t - r) & 256) >> 4; r -= (t & ((t - r) >> 8));
  t  = (c >> (s - 8)) & 0xf;
  // if (r > t) {s -= 8; r -= t;}
  s -= ((t - r) & 256) >> 5; r -= (t & ((t - r) >> 8));
  t  = (b >> (s - 4)) & 0x7;
  // if (r > t) {s -= 4; r -= t;}
  s -= ((t - r) & 256) >> 6; r -= (t & ((t - r) >> 8));
  t  = (a >> (s - 2)) & 0x3;
  // if (r > t) {s -= 2; r -= t;}
  s -= ((t - r) & 256) >> 7; r -= (t & ((t - r) >> 8));
  t  = (v >> (s - 1)) & 0x1;
  // if (r > t) s--;
  s -= ((t - r) & 256) >> 8;
  s = 65 - s;
  return s;
}

uint64_t count_bits_set(uint64_t v) {
    v = v - ((v >> 1) & (uint64_t)~(uint64_t)0/3);                                          // temp
    v = (v & (uint64_t)~(uint64_t)0/15*3) + ((v >> 2) & (uint64_t)~(uint64_t)0/15*3);       // temp
    v = (v + (v >> 4)) & (uint64_t)~(uint64_t)0/255*15;                                     // temp
    uint64_t c = (uint64_t)(v * ((uint64_t)~(uint64_t)0/255)) >> (sizeof(uint64_t) - 1) * 8;  // count
    return c;
}

template<typename T, int COUNT, typename HELPER=SimpleHelper<T> >
struct IntSet {
    IntSet() { reset(); }

    void reset() {
        _size = 0;
        memset(chunks, 0, sizeof(chunks));
    }

    void add(T v) {
        int offset = HELPER::MapToNat(v);
        int chunk = offset/64;
        int bit = offset%64;

        if(!(chunks[chunk] & (uint64_t(1)<<bit))) {
            _size++;
            chunks[chunk] |= uint64_t(1)<<bit;
        }
    }

    void remove(T v) {
        int offset = HELPER::MapToNat(v);
        int chunk = offset/64;
        int bit = offset%64;

        if(chunks[chunk] & (uint64_t(1)<<bit)) {
            chunks[chunk] ^= (uint64_t(1)<<bit);
            _size--;
        }
    }

    void addAll(IntSet& o) {
        _size = 0;
        for(int c=0; c<CHUNK_COUNT; c++) {
            chunks[c] |= o.chunks[c];
            _size += count_bits_set(chunks[c]);
        }
    }

    bool contains(T v) {
        int offset = HELPER::MapToNat(v);
        int chunk = offset/64;
        int bit = offset%64;

        if(chunks[chunk] & (uint64_t(1)<<bit)) {
            return true;
        }
        return false;
    }

/*    void dump() {
        printf("{");
        for(T i=0; i<_size; i++) {
            T v = _list[i];
            printf("(%d,%d)", X(p)-1, Y(p)-1);
            if(i<(_size-1)) {
                printf(", ");
            }
        }
        printf("}");
    }
*/

    void getValues(T* out) {
        T* o = out;
        for(int c=0; c<CHUNK_COUNT; c++) {
            uint64_t chunk = chunks[c];
            for(int n=0; n<64; n++) {
                if(chunk & (uint64_t(1)<<n)) {
                    *o++ = HELPER::MapFromNat(c*64+n);
                }
            }
        }
        *o++ = T(-1);
    }

    T size() { return _size; }

    static const int COUNT = COUNT;
    static const int CHUNK_COUNT = (COUNT + 63) / 64;
    uint64_t chunks[CHUNK_COUNT];
    T _size;
};


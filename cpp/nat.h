#pragma once

//typesafe bounded uint
template<class T, class POD=unsigned int>
struct Nat {
    typedef POD pod;

    static T fromUint(pod v) { return T(v); }
    static T invalid() { return T(-1); }

    pod toUint() const { return v; }
    bool isValid() const { return v != pod(-1); }
    bool operator==(const Nat& r) const { return v == r.v; }
    bool operator!=(const Nat& r) const { return v != r.v; }

    bool inc() {
        ++v;
        return v < T::kBound;
    }

    pod v;

protected:
    explicit Nat(pod v) : v(v) {
        ASSERT(pod(-1) > 0); //unsigned
        ASSERT(v < T::kBound || v == pod(-1));
    }
};

#define ForEachNat(N, v) for(N v = N::invalid(); v.inc(); )

#define PRIVATE_NAT_CONSTRUCTOR(T) \
    private: \
        explicit T(uint v) : Nat<T>(v) {} \
        friend struct Nat<T>; \
    public:


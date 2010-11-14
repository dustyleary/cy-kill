#pragma once

//typesafe bounded uint
template<class T, class POD=unsigned int>
struct Nat {
    typedef POD pod;

    static T fromUint(pod v) { return T(v); }
    static T invalid() { return T(-1); }

    pod toUint() const { return v; }
    bool isValid() const { return v < T::kBound; }
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
    }
};

#define FOREACH_NAT(N, v, block) for(N v = N::invalid(); v.inc(); ) block

#define PRIVATE_NAT_CONSTRUCTOR(T) \
    private: \
        explicit T(uint v) : Nat<T>(v) {} \
        friend struct Nat<T>; \
    public: \
        T() : Nat<T>(Nat<T>::pod(-1)) {}


#pragma once

template<typename T>
struct SimpleHelper {
    static T NatMap_for_IntSet(T v) { return v; }
};

template<typename T, int COUNT, typename HELPER=SimpleHelper<T> >
struct IntSet {
    typedef typename T::pod Tpod;
    IntSet() : _size(0) {}

    void reset() {
        _size = 0;
    }

    void add(T v) {
        Tpod offset = HELPER::NatMap_for_IntSet(v);
        Tpod i = _indexes[offset];
        if(i>=0 && i<_size && _list[i]==v) {
            return;
        }
        i = _size++;
        _list[i] = v;
        _indexes[offset] = i;
    }

    void remove(T v) {
        Tpod offset = HELPER::NatMap_for_IntSet(v);
        Tpod i = _indexes[offset];
        if(i>=_size || i<0 || _list[i]!=v) {
            return;
        }
        std::swap(_list[i], _list[_size-1]);
        _size--;
        offset = HELPER::NatMap_for_IntSet(_list[i]);
        _indexes[offset] = i;
    }

    void addAll(IntSet& o) {
        for(Tpod i=0; i<o._size; i++) {
            add(o._list[i]);
        }
    }

    bool contains(T v) const {
        Tpod offset = HELPER::NatMap_for_IntSet(v);
        Tpod i = _indexes[offset];
        if(i>=0 && i<_size && _list[i]==v) {
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

    Tpod size() const { return _size; }
    T& operator[](uint i) {
        ASSERT(i < COUNT);
        return _list[i];
    }
    const T& operator[](uint i) const {
        ASSERT(i < COUNT);
        return _list[i];
    }

    Tpod _size;
    Tpod _indexes[COUNT];
    T _list[COUNT];
};


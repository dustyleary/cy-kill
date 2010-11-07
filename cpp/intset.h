#pragma once

template<typename T>
struct SimpleHelper {
    static T NatMap(T v) { return v; }
};

template<typename T, int COUNT, typename HELPER=SimpleHelper<T> >
struct IntSet {
    IntSet() : _size(0) {}

    void reset() {
        _size = 0;
    }

    void add(T v) {
        T offset = HELPER::NatMap(v);
        T i = _indexes[offset];
        if(i>=0 && i<_size && _list[i]==v) {
            return;
        }
        i = _size++;
        _list[i] = v;
        _indexes[offset] = i;
    }

    void remove(T v) {
        T offset = HELPER::NatMap(v);
        T i = _indexes[offset];
        if(i>=_size || i<0 || _list[i]!=v) {
            return;
        }
        std::swap(_list[i], _list[_size-1]);
        _size--;
        offset = HELPER::NatMap(_list[i]);
        _indexes[offset] = i;
    }

    void addAll(IntSet& o) {
        for(T i=0; i<o._size; i++) {
            add(o._list[i]);
        }
    }

    bool contains(T v) {
        T offset = HELPER::NatMap(v);
        T i = _indexes[offset];
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

    T size() { return _size; }

    T _size;
    T _indexes[COUNT];
    T _list[COUNT];
};


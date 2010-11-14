#pragma once

template<typename Nat>
struct NatSet {
    NatSet() : _size(0) {}

    void reset() {
        _size = 0;
    }

    uint size() const {
        return _size;
    }

    bool contains(Nat n) const {
        uint i = _indexes[n];
        return (i<_size && _list[i]==n);
    }

    void add(Nat n) {
        uint i = _indexes[n];
        if(i<_size && _list[i]==n) {
            return;
        }
        i = _size++;
        _list[i] = n;
        _indexes[n] = i;
    }

    void remove(Nat n) {
        uint i = _indexes[n];
        if(i>=_size || _list[i]!=n) {
            return;
        }
        std::swap(_list[i], _list[_size-1]);
        _size--;
        _indexes[_list[i]] = i;
    }

    const Nat& operator[](uint i) const {
        return _list[i];
    }

private:
    uint _size;
    NatMap<Nat, uint> _indexes;
    Nat _list[Nat::kBound];
};


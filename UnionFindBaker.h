//
// Created by Usuario on 6/11/2026.
//

#ifndef UNIONFINDBAKER_H
#define UNIONFINDBAKER_H
#include <vector>


struct PersistentArray {
private:
    std::vector<int> array;
    PersistentArray() = default;
    explicit PersistentArray(std::vector<int> v):array(std::move(v)){};
public:
    static PersistentArray init(int n,int(*operacion)(int));
    int get(int indice) const;
    PersistentArray set(int index,int value) const;
};

// union find persistente
// acepta cualquier implementacion de array persistente
template <typename Array = PersistentArray>
class UnionFindBaker {
private:
    Array rank;
    mutable Array parent;
    UnionFindBaker Link(int x,int y) const;
    std::tuple<int,Array> find_aux(Array f,int i);
public:
    explicit UnionFindBaker(int n);
    int Find(int x);
    UnionFindBaker Union(int x,int y);
};


#endif //UNIONFINDBAKER_H

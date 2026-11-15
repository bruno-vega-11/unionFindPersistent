//
// Created by Usuario on 6/11/2026.
//

#ifndef UNIONFINDBAKER_H
#define UNIONFINDBAKER_H
#include <memory>
#include <random>
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


struct Node {
    bool isArr = true;
    std::vector<int> arr;
    int i = 0, v = 0;
    std::shared_ptr<Node> next = nullptr;
};

struct PersistentArrayBaker {
private:
    std::shared_ptr<Node> node;
    PersistentArrayBaker(std::shared_ptr<Node> p): node(std::move(p)){};
    static void reroot(const std::shared_ptr<Node> t);
public:
    static PersistentArrayBaker init(int n,int(*operacion)(int));
    int get(int indice) const;
    PersistentArrayBaker set(int index,int value) const;
};


// union find persistente
// acepta cualquier implementacion de array persistente
template <typename Array = PersistentArrayBaker>
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

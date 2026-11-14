//
// Parte 4: union find persistente sobre segment tree persistente
//

#ifndef UNIONFINDSEGTREE_H
#define UNIONFINDSEGTREE_H
#include <tuple>
#include "PersistentArraySegTree.h"

// mismo algoritmo que UnionFindBaker (rank + path compression, Conchon y Filliatre)
// pero con el arreglo persistente hecho con segment tree
// (analogo a la variante con arboles AVL del paper)
class UnionFindSegTree {
private:
    PersistentArraySegTree rank;
    mutable PersistentArraySegTree parent;
    UnionFindSegTree(PersistentArraySegTree rank, PersistentArraySegTree parent);
    UnionFindSegTree Link(int x,int y) const;
    std::tuple<int,PersistentArraySegTree> find_aux(PersistentArraySegTree f,int i) const;
public:
    explicit UnionFindSegTree(int n);
    int Find(int x) const;
    UnionFindSegTree Union(int x,int y) const;
    void Print() const;
};

#endif //UNIONFINDSEGTREE_H

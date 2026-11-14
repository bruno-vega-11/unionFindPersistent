//
// Parte 4: union find persistente sobre segment tree persistente
//

#include "UnionFindSegTree.h"
#include <iostream>

UnionFindSegTree::UnionFindSegTree(int n) {
    rank = PersistentArraySegTree::init(n,[](int){return 0;});
    parent = PersistentArraySegTree::init(n,[](int i){return i;});
}

UnionFindSegTree::UnionFindSegTree(PersistentArraySegTree rank, PersistentArraySegTree parent)
    :rank(rank),parent(parent){}

// se asume que "x" y "y" son los representantes
// de distintos set
UnionFindSegTree UnionFindSegTree::Link(int x, int y) const {
    int rx = rank.get(x);
    int ry = rank.get(y);

    if (rx > ry) {
        PersistentArraySegTree nuevo_parent = parent.set(y,x);
        return UnionFindSegTree(rank,nuevo_parent);
    } else {
        PersistentArraySegTree nuevo_parent = parent.set(x,y);
        if (rx == ry) {
            PersistentArraySegTree nuevo_rank = rank.set(y,ry + 1);
            return UnionFindSegTree(nuevo_rank,nuevo_parent);
        }
        return UnionFindSegTree(rank,nuevo_parent);
    }
}

// devuelve (representante, parent con el camino comprimido)
std::tuple<int, PersistentArraySegTree> UnionFindSegTree::find_aux(PersistentArraySegTree parent_array, int i) const {
    int parent_now = parent_array.get(i);

    // su padre es él mismo
    if (parent_now == i) {
        return {i,parent_array};
    }
    // recursion
    std::tuple<int,PersistentArraySegTree> resultado = find_aux(parent_array,parent_now);
    // path compression
    PersistentArraySegTree compression = std::get<PersistentArraySegTree>(resultado).set(i,std::get<int>(resultado));

    return {std::get<int>(resultado),compression};
}

// la compresion se guarda en esta misma version (parent es mutable):
// no cambia la particion, asi que la persistencia es observacional
int UnionFindSegTree::Find(int x) const {
    std::tuple<int,PersistentArraySegTree> resultado = find_aux(parent,x);
    parent = std::get<PersistentArraySegTree>(resultado);
    return std::get<int>(resultado);
}

// devuelve una version nueva; la actual no cambia
UnionFindSegTree UnionFindSegTree::Union(int x, int y) const {
    int cx = Find(x);
    int cy = Find(y);
    if (cx == cy) {
        return *this;
    }
    return Link(cx,cy);
}

void UnionFindSegTree::Print() const {
    std::cout << "parents: ";
    for (int i = 0; i < parent.size(); i++) {
        std::cout << parent.get(i) << " ";
    }
}

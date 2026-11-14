//
// Parte 5: union find con rollback
//

#include "UnionFindRollback.h"
#include <iostream>

UnionFindRollback::UnionFindRollback(int n) {
    parent.resize(n);
    rank.resize(n, 0);
    for (int i = 0; i < n;i++) {
        parent[i] = i;
    }
}

void UnionFindRollback::Union(int x, int y) {
    int cx = Find(x);
    int cy = Find(y);
    if (cx != cy) {
        Link(cx, cy);
    }
}

// se asume que "x" y "y" son los representantes
// de distintos set
void UnionFindRollback::Link(int x, int y) {
    if (rank[x] > rank[y]) {
        parent[y] = x;
        historial.push_back({y, x, false});
    } else {
        parent[x] = y;
        bool subio = (rank[x] == rank[y]);
        if (subio) {
            rank[y] += 1;
        }
        historial.push_back({x, y, subio});
    }
}

// sin path compression: con union by rank la altura es O(log n)
// y cada Union cambia solo O(1) posiciones, que es lo que permite deshacerla
int UnionFindRollback::Find(int x) {
    while (parent[x] != x) {
        x = parent[x];
    }
    return x;
}

void UnionFindRollback::Print() {
    std::cout << "parents: ";
    for (int p : parent) {
        std::cout << p << " ";
    }
}

// momento al que luego se puede volver
int UnionFindRollback::Checkpoint() const {
    return static_cast<int>(historial.size());
}

// deshace las uniones hechas despues del checkpoint, O(1) cada una
void UnionFindRollback::Rollback(int checkpoint) {
    while (static_cast<int>(historial.size()) > checkpoint) {
        Cambio c = historial.back();
        historial.pop_back();
        parent[c.hijo] = c.hijo;
        if (c.subio_rank) {
            rank[c.padre] -= 1;
        }
    }
}

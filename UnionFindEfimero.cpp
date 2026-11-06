//
// Created by Usuario on 6/11/2026.
//

#include "UnionFindEfimero.h"
#include <iostream>

UnionFindEfimero::UnionFindEfimero(const std::vector<int>& y) {
    parent.resize(y.size());
    rank.resize(y.size(), 0);
    for (int i = 0; i < y.size();i++) {
        parent[i] = i;
    }
}

void UnionFindEfimero::Union(int x, int y) {
    Link(Find(x), Find(y));
}

// se asume que "x" y "y" son los representantes
// de distintos set
void UnionFindEfimero::Link(int x, int y) {
    if (rank[x] > rank[y]) {
        parent[y] = x;
    } else {
        parent[x] = y;
        if (rank[x] == rank[y]) {
            rank[y] += 1;
        }
    }
}

int UnionFindEfimero::Find(int x) {
    if (parent[x] != x) {
        parent[x] = Find(parent[x]);
    }
    return parent[x];
}

void UnionFindEfimero::Print() {
    std::cout << "parents: ";
    for (int p : parent) {
        std::cout << p << " ";
    }
}
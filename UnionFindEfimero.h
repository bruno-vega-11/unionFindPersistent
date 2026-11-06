//
// Created by Usuario on 6/11/2026.
//

#ifndef UNIONFINDEFIMERO_H
#define UNIONFINDEFIMERO_H
#include <vector>
#include "UnionFind.h"

class UnionFindEfimero:public UnionFind{
private:
    std::vector<int> parent;
    std::vector<int> rank;
public:
    explicit UnionFindEfimero(const std::vector<int>& y);
    void Union(int x, int y) override;
    void Link(int x, int y) override;
    int Find(int x) override;
    void Print() override;
};

#endif //UNIONFINDEFIMERO_H

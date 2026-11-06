//
// Created by Usuario on 6/11/2026.
//

#ifndef UNIONFIND_H
#define UNIONFIND_H

// se asume que se mantiene elementos
// del 0 al N-1

// clase abstracta
class UnionFind {
public:
    virtual void Union(int x,int y) = 0;
    virtual void Link(int x,int y) = 0;
    virtual int Find(int x) = 0;
    virtual void Print() = 0;
    virtual ~UnionFind()=default;
};


#endif //UNIONFIND_H

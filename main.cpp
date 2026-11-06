#include <iostream>
#include <vector>

using namespace std;

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

// union find efimero (baseline)
class UnionFindEfimero: public UnionFind {
private:
    vector<int> parent;
    vector<int> rank;
public:
    explicit UnionFindEfimero(const vector<int>& y) {
        parent.resize(y.size());
        rank.resize(y.size(),0);
        for (int i = 0; i < y.size();i++) {
            parent[i] = i;
        }
    }

    void Union(int x, int y) override {
        Link(Find(x),Find(y));
    }

    // se asume que "x" y "y" son los representantes
    // de distintos set
    void Link(int x, int y) override {
        if (rank[x] > rank[y]) {
            parent[y] = x;
        } else {
            parent[x] = y;
            if (rank[x] == rank[y]) {
                rank[y] += 1;
            }
        }
    }

    int Find(int x) override {
        if (parent[x] != x) {
            parent[x] = Find(parent[x]);
        }
        return parent[x];
    }

    void Print() override {
        cout << "parents:";
        for (int i : parent) {
            cout << i << " ";
        }
    }
};

// union find persistente sobre arreglo Baker
class UnionFindBaker: public UnionFind {

};

int main() {
    vector<int> prueba = {0,1,2,3,4,5,6,7,8,9,10};
    UnionFindEfimero efimero(prueba);
    efimero.Union(0,10);
    efimero.Union(1,9);
    efimero.Union(2,8);
    efimero.Union(0,8);
    efimero.Union(0,9);
    efimero.Print();
};
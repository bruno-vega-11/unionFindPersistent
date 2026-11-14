//
// Parte 5: union find con rollback
//

#ifndef UNIONFINDROLLBACK_H
#define UNIONFINDROLLBACK_H
#include <vector>
#include "UnionFind.h"

// union by rank SIN path compression y una pila con los cambios
// no es persistente: solo se puede volver atras en orden (backtracking)
class UnionFindRollback:public UnionFind{
private:
    struct Cambio {
        int hijo;          // representante que se colgo debajo de otro
        int padre;         // representante que quedo arriba
        bool subio_rank;   // si al padre se le aumento el rank
    };
    std::vector<int> parent;
    std::vector<int> rank;
    std::vector<Cambio> historial;
public:
    explicit UnionFindRollback(int n);
    void Union(int x, int y) override;
    void Link(int x, int y) override;
    int Find(int x) override;
    void Print() override;
    int Checkpoint() const;
    void Rollback(int checkpoint);
};

#endif //UNIONFINDROLLBACK_H

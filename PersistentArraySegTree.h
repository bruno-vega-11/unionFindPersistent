//
// Parte 4: arreglo persistente con segment tree (path copying)
//

#ifndef PERSISTENTARRAYSEGTREE_H
#define PERSISTENTARRAYSEGTREE_H
#include <vector>

// misma interfaz que PersistentArray (init, get, set)
// get y set cuestan O(log n); set devuelve una version nueva y no modifica la actual
class PersistentArraySegTree {
private:
    struct Nodo {
        int valor;    // solo se usa en las hojas
        int izq, der; // indices en el pool (-1 si es hoja)
    };
    // todos los arreglos comparten el mismo pool de nodos
    static std::vector<Nodo> nodos;

    int n = 0;
    int raiz = -1;
    PersistentArraySegTree(int n, int raiz):n(n),raiz(raiz){};

    static int nuevo(int valor, int izq, int der);
    static int build(int l, int r, int(*operacion)(int));
    static int set_aux(int nodo, int l, int r, int index, int value);
public:
    PersistentArraySegTree() = default;
    static PersistentArraySegTree init(int n,int(*operacion)(int));
    int get(int indice) const;
    PersistentArraySegTree set(int index,int value) const;
    int size() const;

    // C++ no tiene GC: en backtracking se puede liberar el pool como una pila
    // ojo: Find comprime caminos con nodos nuevos; antes de Marca() guardar una copia
    // de la version y restaurarla tras Liberar()
    static size_t Marca();
    static void Liberar(size_t marca);
    // vacia el pool y devuelve su memoria (invalida todas las versiones existentes)
    static void LiberarTodo();
};

#endif //PERSISTENTARRAYSEGTREE_H

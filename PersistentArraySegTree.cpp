//
// Parte 4: arreglo persistente con segment tree (path copying)
//

#include "PersistentArraySegTree.h"

std::vector<PersistentArraySegTree::Nodo> PersistentArraySegTree::nodos;

int PersistentArraySegTree::nuevo(int valor, int izq, int der) {
    nodos.push_back({valor, izq, der});
    return static_cast<int>(nodos.size()) - 1;
}

int PersistentArraySegTree::build(int l, int r, int (*operacion)(int)) {
    if (l == r) {
        return nuevo(operacion(l), -1, -1);
    }
    int m = (l + r) / 2;
    int izq = build(l, m, operacion);
    int der = build(m + 1, r, operacion);
    return nuevo(0, izq, der);
}

PersistentArraySegTree PersistentArraySegTree::init(int n, int (*operacion)(int)) {
    return PersistentArraySegTree(n, build(0, n - 1, operacion));
}

// bajar desde la raiz hasta la hoja
int PersistentArraySegTree::get(int indice) const {
    int nodo = raiz;
    int l = 0, r = n - 1;
    while (l < r) {
        int m = (l + r) / 2;
        if (indice <= m) {
            nodo = nodos[nodo].izq;
            r = m;
        } else {
            nodo = nodos[nodo].der;
            l = m + 1;
        }
    }
    return nodos[nodo].valor;
}

// path copying: solo se copian los nodos del camino raiz-hoja
// si el valor ya era igual no se copia nada (optimizacion (a) del paper)
int PersistentArraySegTree::set_aux(int nodo, int l, int r, int index, int value) {
    if (l == r) {
        if (nodos[nodo].valor == value) {
            return nodo;
        }
        return nuevo(value, -1, -1);
    }
    int m = (l + r) / 2;
    int izq = nodos[nodo].izq;
    int der = nodos[nodo].der;
    if (index <= m) {
        int nuevo_izq = set_aux(izq, l, m, index, value);
        if (nuevo_izq == izq) {
            return nodo;
        }
        return nuevo(0, nuevo_izq, der);
    } else {
        int nuevo_der = set_aux(der, m + 1, r, index, value);
        if (nuevo_der == der) {
            return nodo;
        }
        return nuevo(0, izq, nuevo_der);
    }
}

PersistentArraySegTree PersistentArraySegTree::set(int index, int value) const {
    return PersistentArraySegTree(n, set_aux(raiz, 0, n - 1, index, value));
}

int PersistentArraySegTree::size() const {
    return n;
}

size_t PersistentArraySegTree::Marca() {
    return nodos.size();
}

// se asume que ninguna version viva usa nodos creados despues de la marca
void PersistentArraySegTree::Liberar(size_t marca) {
    nodos.resize(marca);
}

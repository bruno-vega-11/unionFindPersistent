//
// Created by Usuario on 6/11/2026.
//

#include "UnionFindBaker.h"


//-------------------------------------PersistentArray-1--------------------------------//

PersistentArray PersistentArray::init(int n, int (*operacion)(int)) {
    std::vector<int> array(n);
    for (int i = 0;i < n;i++) {
        array[i] = operacion(i);
    }
    return PersistentArray(array);
}

int PersistentArray::get(int indice) const {
    return array[indice];
}

PersistentArray PersistentArray::set(int index, int value) const {
    std::vector<int> new_array = array;
    new_array[index] = value;
    return PersistentArray(new_array);
}

//----------------------------------PersistentArray-Baker--------------------------------//

Node::~Node() {
    // se "roba" el siguiente nodo antes de que muera este, para que la cadena
    // se libere en un bucle y no con una llamada recursiva por nodo
    std::shared_ptr<Node> p = std::move(next);
    while (p && p.use_count() == 1) {
        std::shared_ptr<Node> q = std::move(p->next);
        p = std::move(q);
    }
}

// version iterativa del reroot del paper (alli se usa CPS para no desbordar la pila):
// primero se recorre la cadena de Diff hasta el Arr y luego se invierte
// desde el final, que es lo mismo que haria la recursion al volver
void PersistentArrayBaker::reroot(const std::shared_ptr<Node>& t) {
    if (t->isArr) return;
    std::vector<std::shared_ptr<Node>> camino;
    for (std::shared_ptr<Node> p = t; ; p = p->next) {
        camino.push_back(p);
        if (p->isArr) break;
    }

    for (int k = static_cast<int>(camino.size()) - 2; k >= 0; k--) {
        const std::shared_ptr<Node>& actual = camino[k];
        const std::shared_ptr<Node>& tp = camino[k + 1];  // ya es Arr
        int i = actual->i;
        int v = actual->v;

        int vp = tp->arr[i];
        tp->arr[i] = v;

        actual->arr = std::move(tp->arr);
        actual->isArr = true;
        actual->next = nullptr;

        tp->arr.clear();
        tp->isArr = false;
        tp->i = i;
        tp->v = vp;
        tp->next = actual;
    }
}


PersistentArrayBaker PersistentArrayBaker::init(int n, int (*operacion)(int)) {
    std::shared_ptr<Node> p = std::make_shared<Node>();
    p->arr.resize(n);
    for (int i = 0;i < n;i++) {
        p->arr[i] = operacion(i);
    }
    return PersistentArrayBaker(p);
}

int PersistentArrayBaker::get(int indice) const {
    reroot(node);
    return node->arr[indice];
}

PersistentArrayBaker PersistentArrayBaker::set(int index, int value) const {
    reroot(node);
    std::shared_ptr<Node> res = std::make_shared<Node>();
    res->arr = std::move(node->arr);

    int old = res->arr[index];
    res->arr[index] = value;

    node->isArr = false;
    node->i = index;
    node->v = old;
    node->next = res;
    return PersistentArrayBaker(res);
}

// la implementacion de UnionFindBaker esta en el .h (es un template)

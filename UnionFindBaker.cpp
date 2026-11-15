//
// Created by Usuario on 6/11/2026.
//

#include "UnionFindBaker.h"

#include <algorithm>

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

void PersistentArrayBaker::reroot(const std::shared_ptr<Node> t) {
    if (t->isArr) return;
    std::shared_ptr<Node> tp = t->next;
    int i = t->i;
    int v = t->v;
    reroot(tp);

    int vp = tp->arr[i];
    tp->arr[i] = v;

    t->arr = std::move(tp->arr);
    t->isArr = true;
    t->next = nullptr;

    tp->isArr = false;
    tp->i = i;
    tp->v = vp;
    tp->next = t;
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

//-------------------------------------UnionFindPersistente--------------------------------//

template<typename Array>
UnionFindBaker<Array>::UnionFindBaker(int n) {
    rank = rank.init(n,[](int){return 0;});
    parent = parent.init(n,[](int i){return i;});
}

template<typename Array>
UnionFindBaker<Array> UnionFindBaker<Array>::Link(int x, int y) const {
    int rx = rank.get(x);
    int ry = rank.get(y);

    if (rx > ry) {
        Array nuevo_parent = parent.set(y,x);
        return UnionFindBaker(rank,nuevo_parent);
    } else {
        Array nuevo_parent = parent.set(x,y);
        if (rx == ry) {
            Array nuevo_rank = rank.set(y,ry + 1);
            return UnionFindBaker(nuevo_rank,nuevo_parent);
        }
        return UnionFindBaker(rank,nuevo_parent);
    }
}


template<typename Array>
std::tuple<int, Array> UnionFindBaker<Array>::find_aux(Array parent_array, int i) {
    int parent_now = parent_array.get(i);

    // su padre es él mismo
    if (parent_now == i) {
        return {parent_array,i};
    }
    // recursion
    std::tuple<int,Array> resultado = find_aux(parent_array,parent_now);
    // path compression
    Array compression = get<Array>(resultado).set(i,get<int>(resultado));

    return {get<int>(resultado),compression};
}


template<typename Array>
int UnionFindBaker<Array>::Find(int x) {
   std::tuple<int,Array> resultado = find_aux(parent,x);
    parent = get<Array>(resultado);
    return get<int>(resultado);
}


template<typename Array>
UnionFindBaker<Array> UnionFindBaker<Array>::Union(int x, int y) {
    int px = parent.get(x);
    int py = parent.get(y);
    if (px == py) {
        return *this;
    }
    return Link(x,y);
}

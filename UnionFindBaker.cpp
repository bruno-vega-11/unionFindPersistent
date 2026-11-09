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

//-------------------------------------UnionFindPersistente--------------------------------//

template<typename Array>
UnionFindBaker<Array>::UnionFindBaker(int n) {
    rank = rank.init(n,[](int){return 0;});
    parent = parent.init(n,[](int i){return i;});
}

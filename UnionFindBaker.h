//
// Created by Usuario on 6/11/2026.
//

#ifndef UNIONFINDBAKER_H
#define UNIONFINDBAKER_H
#include <memory>
#include <tuple>
#include <vector>


// arreglo persistente ingenuo: copia todo en cada set (O(n))
// sirve como referencia para las pruebas
struct PersistentArray {
private:
    std::vector<int> array;
    explicit PersistentArray(std::vector<int> v):array(std::move(v)){};
public:
    PersistentArray() = default;
    static PersistentArray init(int n,int(*operacion)(int));
    int get(int indice) const;
    PersistentArray set(int index,int value) const;
};


struct Node {
    bool isArr = true;
    std::vector<int> arr;
    int i = 0, v = 0;
    std::shared_ptr<Node> next = nullptr;
    Node() = default;
    // destruccion iterativa: con el destructor por defecto una cadena larga
    // de Diff se destruye recursivamente y desborda la pila
    ~Node();
};

struct PersistentArrayBaker {
private:
    std::shared_ptr<Node> node;
    PersistentArrayBaker(std::shared_ptr<Node> p): node(std::move(p)){};
    static void reroot(const std::shared_ptr<Node>& t);
public:
    PersistentArrayBaker() = default;
    static PersistentArrayBaker init(int n,int(*operacion)(int));
    int get(int indice) const;
    PersistentArrayBaker set(int index,int value) const;
};


// variante 3: arreglo de Baker con la optimizacion (a) del paper
// set(t,i,v) no crea una version nueva si t[i] ya vale v
// (muy frecuente en path compression: el nodo ya apunta a la raiz)
struct PersistentArrayBakerOpt {
private:
    PersistentArrayBaker base;
    explicit PersistentArrayBakerOpt(PersistentArrayBaker b):base(std::move(b)){};
public:
    PersistentArrayBakerOpt() = default;
    static PersistentArrayBakerOpt init(int n,int(*operacion)(int)) {
        return PersistentArrayBakerOpt(PersistentArrayBaker::init(n,operacion));
    }
    int get(int indice) const { return base.get(indice); }
    PersistentArrayBakerOpt set(int index,int value) const {
        if (base.get(index) == value) {
            return *this;
        }
        return PersistentArrayBakerOpt(base.set(index,value));
    }
};


// union find persistente (Conchon y Filliatre)
// UnionFindBaker<>                        -> variante 2
// UnionFindBaker<PersistentArrayBakerOpt> -> variante 3 (optimizacion (a))
// acepta cualquier implementacion de array persistente con init, get y set
// (por ser template, la implementacion tiene que estar en el .h)
template <typename Array = PersistentArrayBaker>
class UnionFindBaker {
private:
    Array rank;
    mutable Array parent;
    UnionFindBaker(Array rank, Array parent):rank(std::move(rank)),parent(std::move(parent)){};
    UnionFindBaker Link(int x,int y) const;
    static std::tuple<int,Array> find_aux(const Array& f,int i);
public:
    explicit UnionFindBaker(int n);
    int Find(int x) const;
    UnionFindBaker Union(int x,int y) const;
};


//-------------------------------------UnionFindPersistente--------------------------------//

template<typename Array>
UnionFindBaker<Array>::UnionFindBaker(int n)
    :rank(Array::init(n,[](int){return 0;})),
     parent(Array::init(n,[](int i){return i;})){}

// se asume que "x" y "y" son los representantes
// de distintos set
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

// devuelve (representante, parent con el camino comprimido)
// la recursion tiene profundidad O(log n) gracias a union by rank
template<typename Array>
std::tuple<int, Array> UnionFindBaker<Array>::find_aux(const Array& parent_array, int i) {
    int parent_now = parent_array.get(i);

    // su padre es él mismo
    if (parent_now == i) {
        return {i,parent_array};
    }
    // recursion
    std::tuple<int,Array> resultado = find_aux(parent_array,parent_now);
    // path compression
    Array compression = std::get<Array>(resultado).set(i,std::get<int>(resultado));

    return {std::get<int>(resultado),compression};
}

// la compresion se guarda en esta misma version (parent es mutable):
// no cambia la particion, asi que la persistencia es observacional
template<typename Array>
int UnionFindBaker<Array>::Find(int x) const {
    std::tuple<int,Array> resultado = find_aux(parent,x);
    parent = std::get<Array>(resultado);
    return std::get<int>(resultado);
}

// devuelve una version nueva; la actual no cambia
template<typename Array>
UnionFindBaker<Array> UnionFindBaker<Array>::Union(int x, int y) const {
    int cx = Find(x);
    int cy = Find(y);
    if (cx == cy) {
        return *this;
    }
    return Link(cx,cy);
}


#endif //UNIONFINDBAKER_H

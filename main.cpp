#include <cassert>
#include <iostream>
#include "UnionFindEfimero.h"
#include "UnionFindBaker.h"
using namespace std;

int main() {
    vector<int> prueba = {0,1,2,3,4,5,6,7,8,9,10};
    UnionFindEfimero efimero(prueba);
    efimero.Union(0,10);
    efimero.Union(1,9);
    efimero.Union(2,8);
    efimero.Union(0,8);
    efimero.Union(0,9);
    efimero.Print();

    auto a0 = PersistentArrayBaker::init(7, [](int){ return 0; });
    auto a1 = a0.set(1, 7);
    auto a2 = a1.set(2, 8);
    auto a3 = a1.set(2, 9);
    cout << "wa" << endl;
    cout << "a0: " << a0.get(1) << " " << a0.get(2) << "\n";   // esperado: 0 0
    cout << "a1: " << a1.get(1) << " " << a1.get(2) << "\n";   // esperado: 7 0
    cout << "a2: " << a2.get(2) << "\n";                       // esperado: 8
    cout << "a3: " << a3.get(2) << "\n";                       // esperado: 9
};
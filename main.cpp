#include <iostream>
#include "UnionFindEfimero.h"
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
};
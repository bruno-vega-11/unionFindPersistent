//
// Pruebas de correctitud contra una implementacion de referencia ingenua
// Cubre: UF efimero (parte 1), UF sobre arreglos de Baker (parte 2),
//        UF persistente con segment tree (parte 4) y UF con rollback (parte 5)
//
// Compilar:  g++ -std=c++20 -O2 tests.cpp UnionFindEfimero.cpp UnionFindBaker.cpp
//                PersistentArraySegTree.cpp UnionFindSegTree.cpp UnionFindRollback.cpp -o tests
//

#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "UnionFindEfimero.h"
#include "UnionFindBaker.h"
#include "PersistentArraySegTree.h"
#include "UnionFindSegTree.h"
#include "UnionFindRollback.h"

// ------------------------------------------------------------------ mini framework

static int fallos = 0;
static int checks = 0;

#define CHECK(cond, msg)                                                         \
    do {                                                                         \
        checks++;                                                                \
        if (!(cond)) {                                                           \
            fallos++;                                                            \
            if (fallos <= 20)                                                    \
                std::cerr << "  FALLO [" << __LINE__ << "] " << msg << "\n";     \
        }                                                                        \
    } while (0)

static void correr(const std::string& nombre, const std::function<void()>& prueba) {
    int antes = fallos;
    prueba();
    std::cout << (fallos == antes ? "[ OK ] " : "[FAIL] ") << nombre << "\n";
}

// ------------------------------------------------------------------ referencia ingenua

// cada elemento guarda la etiqueta de su conjunto; Union reetiqueta en O(n)
struct Referencia {
    std::vector<int> etiqueta;
    explicit Referencia(int n) : etiqueta(n) { std::iota(etiqueta.begin(), etiqueta.end(), 0); }
    void Union(int x, int y) {
        int ex = etiqueta[x], ey = etiqueta[y];
        if (ex == ey) return;
        for (int& e : etiqueta)
            if (e == ey) e = ex;
    }
    int size() const { return static_cast<int>(etiqueta.size()); }
};

// verifica que "find" induce exactamente la misma particion que la referencia:
//  - find(x) pertenece al conjunto de x
//  - find(x) == find(y)  <=>  x e y estan en el mismo conjunto
static bool mismaParticion(const Referencia& ref, const std::function<int(int)>& find) {
    std::map<int, int> rep_de_etiqueta, etiqueta_de_rep;
    for (int x = 0; x < ref.size(); x++) {
        int r = find(x);
        if (r < 0 || r >= ref.size()) return false;
        int e = ref.etiqueta[x];
        if (ref.etiqueta[r] != e) return false;
        auto [it1, nuevo1] = rep_de_etiqueta.emplace(e, r);
        if (!nuevo1 && it1->second != r) return false;
        auto [it2, nuevo2] = etiqueta_de_rep.emplace(r, e);
        if (!nuevo2 && it2->second != e) return false;
    }
    return true;
}

// captura lo que imprime Print() y devuelve el arreglo parent
template <typename F>
static std::vector<int> capturarParents(F imprimir) {
    std::ostringstream ss;
    std::streambuf* viejo = std::cout.rdbuf(ss.rdbuf());
    imprimir();
    std::cout.rdbuf(viejo);
    std::istringstream in(ss.str());
    std::string basura;
    in >> basura;  // "parents:"
    std::vector<int> res;
    int v;
    while (in >> v) res.push_back(v);
    return res;
}

// profundidad maxima de los arboles descritos por parent
static int alturaMaxima(const std::vector<int>& parent) {
    int h = 0;
    for (int x = 0; x < static_cast<int>(parent.size()); x++) {
        int d = 0, y = x;
        while (parent[y] != y && d <= static_cast<int>(parent.size())) {
            y = parent[y];
            d++;
        }
        h = std::max(h, d);
    }
    return h;
}

static int log2piso(int n) {
    int k = 0;
    while ((1 << (k + 1)) <= n) k++;
    return k;
}

// ================================================================== Parte 1: efimero

static void pruebaEfimeroAleatorio() {
    for (int semilla = 0; semilla < 50; semilla++) {
        std::mt19937 rng(semilla);
        int n = 1 + rng() % 60;
        UnionFindEfimero uf{std::vector<int>(n)};
        Referencia ref(n);
        for (int op = 0; op < 4 * n; op++) {
            int x = rng() % n, y = rng() % n;
            uf.Union(x, y);
            ref.Union(x, y);
            CHECK(mismaParticion(ref, [&](int i) { return uf.Find(i); }),
                  "efimero: particion distinta (semilla " << semilla << ")");
        }
    }
}

// Union sobre el mismo conjunto no debe alterar nada (ni rank)
static void pruebaEfimeroUnionRedundante() {
    // si Union(x,x) subiera el rank de x, un singleton con rank inflado quedaria
    // como raiz al unirse con un arbol grande, y se podria formar una cadena de altura n-1
    int n = 64;
    UnionFindEfimero uf{std::vector<int>(n)};
    for (int i = 0; i < n; i++)
        for (int k = 0; k <= i; k++) uf.Union(i, i);
    for (int i = 1; i < n; i++) uf.Union(i - 1, i);
    std::vector<int> p = capturarParents([&] { uf.Print(); });  // Print no comprime caminos
    CHECK(alturaMaxima(p) <= log2piso(n),
          "efimero: altura " << alturaMaxima(p) << " > log2(n) tras uniones redundantes");
}

// ================================================================== Parte 2: arreglo de Baker

static void pruebaArregloBaker() {
    for (int semilla = 0; semilla < 30; semilla++) {
        std::mt19937 rng(1000 + semilla);
        int n = 1 + rng() % 20;
        std::vector<PersistentArrayBaker> versiones = {PersistentArrayBaker::init(n, [](int i) { return 10 * i; })};
        std::vector<std::vector<int>> ref = {std::vector<int>(n)};
        for (int i = 0; i < n; i++) ref[0][i] = 10 * i;

        for (int op = 0; op < 300; op++) {
            int v = rng() % versiones.size();
            if (rng() % 2) {
                int i = rng() % n, val = rng() % 100;
                versiones.push_back(versiones[v].set(i, val));
                ref.push_back(ref[v]);
                ref.back()[i] = val;
            } else {
                int i = rng() % n;
                CHECK(versiones[v].get(i) == ref[v][i], "baker: get distinto (semilla " << semilla << ")");
            }
        }
        for (size_t v = 0; v < versiones.size(); v++)
            for (int i = 0; i < n; i++)
                CHECK(versiones[v].get(i) == ref[v][i], "baker: version " << v << " corrupta");
    }
}

// cadenas largas de Diff: reroot y la destruccion de la cadena deben ser iterativos
// (con recursion, unos 100k Diff ya desbordan la pila)
static void pruebaArregloBakerCadenaLarga() {
    const int k = 1000000;
    auto a0 = PersistentArrayBaker::init(10, [](int) { return 0; });
    {
        auto a = a0;
        for (int i = 0; i < k; i++) a = a.set(i % 10, i);
        CHECK(a.get(3) == k - 7, "baker: ultima version incorrecta");
        CHECK(a0.get(3) == 0, "baker: reroot hacia una version muy antigua incorrecto");
        CHECK(a.get(3) == k - 7, "baker: reroot de vuelta a la ultima version incorrecto");
    }
    // al salir del bloque se destruye la cadena de un millon de nodos
    CHECK(a0.get(9) == 0, "baker: version antigua corrupta tras destruir la cadena");
}

// ================================================================== Parte 4: segment tree

static void pruebaArregloSegTree() {
    for (int semilla = 0; semilla < 30; semilla++) {
        std::mt19937 rng(2000 + semilla);
        int n = 1 + rng() % 40;
        std::vector<PersistentArraySegTree> versiones = {PersistentArraySegTree::init(n, [](int i) { return 10 * i; })};
        std::vector<std::vector<int>> ref = {std::vector<int>(n)};
        for (int i = 0; i < n; i++) ref[0][i] = 10 * i;

        for (int op = 0; op < 400; op++) {
            int v = rng() % versiones.size();
            if (rng() % 2) {
                int i = rng() % n, val = rng() % 100;
                versiones.push_back(versiones[v].set(i, val));
                ref.push_back(ref[v]);
                ref.back()[i] = val;
            } else {
                int i = rng() % n;
                CHECK(versiones[v].get(i) == ref[v][i], "segtree: get distinto (semilla " << semilla << ")");
            }
        }
        for (size_t v = 0; v < versiones.size(); v++) {
            CHECK(versiones[v].size() == n, "segtree: size incorrecto");
            for (int i = 0; i < n; i++)
                CHECK(versiones[v].get(i) == ref[v][i], "segtree: version " << v << " corrupta");
        }
    }
}

// optimizacion (a): set con el mismo valor no crea nodos
static void pruebaSegTreeSetRedundante() {
    auto a = PersistentArraySegTree::init(100, [](int i) { return i; });
    size_t antes = PersistentArraySegTree::Marca();
    auto b = a.set(37, 37);
    CHECK(PersistentArraySegTree::Marca() == antes, "segtree: set redundante creo nodos");
    CHECK(b.get(37) == 37, "segtree: set redundante cambio el valor");
}

// ================================================================== UF persistente (generico)
// sirven para cualquier variante persistente: Baker (partes 2 y 3) y segment tree (parte 4)

// persistencia total: uniones sobre cualquier version, Find sobre cualquier version
template <typename UF>
static void pruebaUFPersistente(const std::string& nombre, int semillaBase) {
    for (int semilla = 0; semilla < 40; semilla++) {
        std::mt19937 rng(semillaBase + semilla);
        int n = 1 + rng() % 40;
        std::vector<UF> versiones = {UF(n)};
        std::vector<Referencia> ref = {Referencia(n)};

        for (int op = 0; op < 200; op++) {
            int v = rng() % versiones.size();
            int x = rng() % n, y = rng() % n;
            int tipo = rng() % 3;
            if (tipo < 2) {
                versiones.push_back(versiones[v].Union(x, y));
                ref.push_back(ref[v]);
                ref.back().Union(x, y);
            } else {
                // Find con path compression sobre una version vieja
                bool mismo_uf = versiones[v].Find(x) == versiones[v].Find(y);
                bool mismo_ref = ref[v].etiqueta[x] == ref[v].etiqueta[y];
                CHECK(mismo_uf == mismo_ref, nombre << ": Find incorrecto (semilla " << semilla << ")");
            }
        }
        // ninguna version fue alterada por las operaciones sobre otras (persistencia observacional)
        for (size_t v = 0; v < versiones.size(); v++)
            CHECK(mismaParticion(ref[v], [&](int i) { return versiones[v].Find(i); }),
                  nombre << ": version " << v << " alterada (semilla " << semilla << ")");
    }
}

// Union devuelve version nueva y deja intacta la original
template <typename UF>
static void pruebaUFNoModificaOriginal(const std::string& nombre) {
    UF v0(6);
    UF v1 = v0.Union(0, 1);
    UF v2 = v1.Union(2, 3);
    UF v3 = v2.Union(1, 3);
    CHECK(v0.Find(0) != v0.Find(1), nombre << ": v0 no debe tener 0~1");
    CHECK(v1.Find(0) == v1.Find(1) && v1.Find(2) != v1.Find(3), nombre << ": v1 incorrecta");
    CHECK(v2.Find(1) != v2.Find(3), nombre << ": v2 no debe tener 1~3");
    CHECK(v3.Find(0) == v3.Find(2), nombre << ": v3 debe tener 0~2");
    CHECK(v3.Find(4) == 4 && v3.Find(5) == 5, nombre << ": 4 y 5 deben seguir solos");
    // Union de elementos ya unidos no cambia nada
    UF v4 = v3.Union(0, 3);
    CHECK(mismaParticion([&] {
        Referencia r(6); r.Union(0, 1); r.Union(2, 3); r.Union(1, 3); return r; }(),
        [&](int i) { return v4.Find(i); }), nombre << ": union redundante cambio la particion");
}

// escenario de backtracking del paper: arbol binario completo de altura 4,
// en cada arista se hacen "ops" operaciones con una proporcion p de uniones
template <typename UF>
static void pruebaUFBacktracking(const std::string& nombre) {
    for (int p : {5, 10, 15}) {
        std::mt19937 rng(9000 + p);
        int n = 300, ops = 200;
        std::function<void(const UF&, const Referencia&, int)> explorar =
            [&](const UF& v, const Referencia& r, int prof) {
                if (prof == 4) return;
                for (int hijo = 0; hijo < 2; hijo++) {
                    UF w = v;
                    Referencia rw = r;
                    for (int k = 0; k < ops; k++) {
                        int x = rng() % n, y = rng() % n;
                        if (static_cast<int>(rng() % 100) < p) {
                            w = w.Union(x, y);
                            rw.Union(x, y);
                        } else {
                            CHECK((w.Find(x) == w.Find(y)) == (rw.etiqueta[x] == rw.etiqueta[y]),
                                  nombre << ": Find incorrecto en backtracking (p=" << p << ")");
                        }
                    }
                    explorar(w, rw, prof + 1);
                    // al volver, la version del padre debe seguir intacta
                    CHECK(mismaParticion(r, [&](int i) { return v.Find(i); }),
                          nombre << ": version padre alterada al volver (p=" << p << ")");
                }
            };
        explorar(UF(n), Referencia(n), 0);
    }
}

// ================================================================== Parte 4: UF segment tree

// escenario de backtracking del paper (arbol binario de altura 4)
// con Marca/Liberar usados de forma segura: se guarda una copia de la version
// antes de la marca y se restaura despues de liberar
static void pruebaUFSegTreeBacktracking() {
    for (int semilla = 0; semilla < 10; semilla++) {
        std::mt19937 rng(4000 + semilla);
        int n = 200, ops = 60;
        std::function<void(UnionFindSegTree&, const Referencia&, int)> explorar =
            [&](UnionFindSegTree& v, const Referencia& r, int prof) {
                if (prof == 4) return;
                for (int hijo = 0; hijo < 2; hijo++) {
                    UnionFindSegTree guardada = v;
                    size_t marca = PersistentArraySegTree::Marca();
                    UnionFindSegTree w = v;
                    Referencia rw = r;
                    for (int k = 0; k < ops; k++) {
                        int x = rng() % n, y = rng() % n;
                        if (rng() % 10 == 0) {
                            w = w.Union(x, y);
                            rw.Union(x, y);
                        } else {
                            CHECK((w.Find(x) == w.Find(y)) == (rw.etiqueta[x] == rw.etiqueta[y]),
                                  "backtracking segtree: Find incorrecto");
                        }
                    }
                    explorar(w, rw, prof + 1);
                    PersistentArraySegTree::Liberar(marca);
                    v = guardada;
                    CHECK(mismaParticion(r, [&](int i) { return v.Find(i); }),
                          "backtracking segtree: version padre alterada al volver");
                }
            };
        UnionFindSegTree raiz(n);
        Referencia r(n);
        explorar(raiz, r, 0);
    }
}

// ================================================================== Parte 5: rollback

// operaciones aleatorias mezclando Union, Find, Checkpoint y Rollback
static void pruebaRollbackAleatorio() {
    for (int semilla = 0; semilla < 60; semilla++) {
        std::mt19937 rng(5000 + semilla);
        int n = 1 + rng() % 50;
        UnionFindRollback uf(n);
        Referencia ref(n);
        // pila de (checkpoint, copia de la referencia)
        std::vector<std::pair<int, Referencia>> pila;

        for (int op = 0; op < 500; op++) {
            int tipo = rng() % 10;
            if (tipo < 4) {
                int x = rng() % n, y = rng() % n;
                uf.Union(x, y);
                ref.Union(x, y);
            } else if (tipo < 6) {
                pila.emplace_back(uf.Checkpoint(), ref);
            } else if (tipo < 8 && !pila.empty()) {
                // volver a un checkpoint cualquiera de la pila (descarta los posteriores)
                int k = rng() % pila.size();
                uf.Rollback(pila[k].first);
                ref = pila[k].second;
                pila.erase(pila.begin() + k, pila.end());
            } else {
                int x = rng() % n, y = rng() % n;
                CHECK((uf.Find(x) == uf.Find(y)) == (ref.etiqueta[x] == ref.etiqueta[y]),
                      "rollback: Find incorrecto (semilla " << semilla << ")");
            }
            CHECK(mismaParticion(ref, [&](int i) { return uf.Find(i); }),
                  "rollback: particion distinta (semilla " << semilla << ", op " << op << ")");
        }
    }
}

// tras Rollback el estado interno (parent y rank) debe ser identico al del checkpoint:
// se compara contra una estructura nueva que solo aplico las uniones sobrevivientes,
// y luego se aplican las mismas uniones a ambas (si rank difiere, parent divergira)
static void pruebaRollbackEstadoExacto() {
    for (int semilla = 0; semilla < 60; semilla++) {
        std::mt19937 rng(6000 + semilla);
        int n = 2 + rng() % 50;
        UnionFindRollback uf(n);
        std::vector<std::pair<int, int>> antes, despues, otra;
        for (int k = 0; k < n; k++) antes.emplace_back(rng() % n, rng() % n);
        for (int k = 0; k < n; k++) despues.emplace_back(rng() % n, rng() % n);
        // "otra" es distinta de "despues": si se reaplicara "despues", un rank mal
        // restaurado podria converger al valor correcto y el error pasaria desapercibido
        for (int k = 0; k < 4 * n; k++) otra.emplace_back(rng() % n, rng() % n);

        for (auto [x, y] : antes) uf.Union(x, y);
        int cp = uf.Checkpoint();
        std::vector<int> parent_cp = capturarParents([&] { uf.Print(); });
        for (auto [x, y] : despues) uf.Union(x, y);
        uf.Rollback(cp);

        CHECK(uf.Checkpoint() == cp, "rollback: Checkpoint no vuelve al valor guardado");
        CHECK(capturarParents([&] { uf.Print(); }) == parent_cp,
              "rollback: parent no se restauro exactamente (semilla " << semilla << ")");

        UnionFindRollback limpio(n);
        for (auto [x, y] : antes) limpio.Union(x, y);
        for (auto [x, y] : otra) {
            uf.Union(x, y);
            limpio.Union(x, y);
        }
        CHECK(capturarParents([&] { uf.Print(); }) == capturarParents([&] { limpio.Print(); }),
              "rollback: rank no se restauro (diverge de una estructura limpia, semilla " << semilla << ")");
    }
}

// sin path compression la eficiencia depende de que la altura sea <= log2(n)
static void pruebaRollbackAltura() {
    for (int semilla = 0; semilla < 30; semilla++) {
        std::mt19937 rng(7000 + semilla);
        int n = 2 + rng() % 500;
        UnionFindRollback uf(n);
        for (int k = 0; k < 3 * n; k++) {
            uf.Union(rng() % n, rng() % n);
            if (k % 7 == 0) uf.Rollback(std::max(0, uf.Checkpoint() - static_cast<int>(rng() % 5)));
        }
        std::vector<int> p = capturarParents([&] { uf.Print(); });
        CHECK(alturaMaxima(p) <= log2piso(n),
              "rollback: altura " << alturaMaxima(p) << " > log2(" << n << ")");
    }
    // peor caso de union by rank: arbol binomial
    int n = 1024;
    UnionFindRollback uf(n);
    for (int paso = 1; paso < n; paso *= 2)
        for (int i = 0; i + paso < n; i += 2 * paso) uf.Union(i, i + paso);
    CHECK(alturaMaxima(capturarParents([&] { uf.Print(); })) == 10, "rollback: arbol binomial con altura != log2(n)");
}

// casos borde
static void pruebaRollbackBordes() {
    UnionFindRollback uf(5);
    CHECK(uf.Checkpoint() == 0, "checkpoint inicial debe ser 0");
    uf.Union(1, 1);
    CHECK(uf.Checkpoint() == 0, "Union(x,x) no debe registrar cambios");
    uf.Union(0, 1);
    uf.Union(1, 0);  // redundante
    CHECK(uf.Checkpoint() == 1, "Union redundante no debe registrar cambios");
    int cp = uf.Checkpoint();
    uf.Rollback(cp);  // rollback al presente: no hace nada
    CHECK(uf.Find(0) == uf.Find(1), "Rollback al presente deshizo algo");
    uf.Union(2, 3);
    uf.Union(3, 4);
    uf.Union(0, 4);
    uf.Rollback(0);  // volver al inicio
    for (int i = 0; i < 5; i++) CHECK(uf.Find(i) == i, "Rollback(0) debe dejar todo como singletons");
    // checkpoints anidados
    uf.Union(0, 1);
    int c1 = uf.Checkpoint();
    uf.Union(2, 3);
    int c2 = uf.Checkpoint();
    uf.Union(0, 2);
    uf.Rollback(c2);
    CHECK(uf.Find(0) != uf.Find(2) && uf.Find(2) == uf.Find(3), "rollback anidado c2 incorrecto");
    uf.Rollback(c1);
    CHECK(uf.Find(2) != uf.Find(3) && uf.Find(0) == uf.Find(1), "rollback anidado c1 incorrecto");
}

// escenario de backtracking del paper: arbol binario completo de altura 4,
// N operaciones entre cada nodo y sus hijos, proporcion p de uniones
static void pruebaRollbackBacktrackingPaper() {
    for (int p : {5, 10, 15}) {
        std::mt19937 rng(8000 + p);
        int n = 300, ops = 200;
        UnionFindRollback uf(n);
        Referencia ref(n);
        std::function<void(int)> explorar = [&](int prof) {
            if (prof == 4) return;
            int cp = uf.Checkpoint();
            Referencia guardada = ref;
            for (int hijo = 0; hijo < 2; hijo++) {
                for (int k = 0; k < ops; k++) {
                    int x = rng() % n, y = rng() % n;
                    if (static_cast<int>(rng() % 100) < p) {
                        uf.Union(x, y);
                        ref.Union(x, y);
                    } else {
                        CHECK((uf.Find(x) == uf.Find(y)) == (ref.etiqueta[x] == ref.etiqueta[y]),
                              "backtracking rollback: Find incorrecto (p=" << p << ")");
                    }
                }
                explorar(prof + 1);
                uf.Rollback(cp);
                ref = guardada;
                CHECK(mismaParticion(ref, [&](int i) { return uf.Find(i); }),
                      "backtracking rollback: estado del padre no restaurado (p=" << p << ")");
            }
        };
        explorar(0);
        CHECK(uf.Checkpoint() == 0, "backtracking rollback: historial no vacio al terminar");
    }
}

// ================================================================== main

int main() {
    std::cout << "Parte 1 - UF efimero\n";
    correr("particion aleatoria vs referencia", pruebaEfimeroAleatorio);
    correr("Union redundante no altera rank", pruebaEfimeroUnionRedundante);

    std::cout << "Parte 2 - UF persistente sobre arreglos de Baker\n";
    correr("arreglo: versiones aleatorias vs copias", pruebaArregloBaker);
    correr("arreglo: cadena de 1M de Diff sin desbordar la pila", pruebaArregloBakerCadenaLarga);
    correr("UF: persistencia total aleatoria", [] { pruebaUFPersistente<UnionFindBaker<>>("uf baker", 3500); });
    correr("UF: Union no modifica la version original", [] { pruebaUFNoModificaOriginal<UnionFindBaker<>>("uf baker"); });
    correr("UF: backtracking del paper (altura 4, p=5/10/15%)", [] { pruebaUFBacktracking<UnionFindBaker<>>("uf baker"); });

    std::cout << "Parte 4 - segment tree persistente\n";
    correr("arreglo: versiones aleatorias vs copias", pruebaArregloSegTree);
    correr("arreglo: set redundante no crea nodos", pruebaSegTreeSetRedundante);
    correr("UF: persistencia total aleatoria", [] { pruebaUFPersistente<UnionFindSegTree>("uf segtree", 3000); });
    correr("UF: Union no modifica la version original", [] { pruebaUFNoModificaOriginal<UnionFindSegTree>("uf segtree"); });
    correr("UF: backtracking del paper (altura 4, p=5/10/15%)", [] { pruebaUFBacktracking<UnionFindSegTree>("uf segtree"); });
    correr("UF: backtracking del paper con Marca/Liberar", pruebaUFSegTreeBacktracking);

    std::cout << "Parte 5 - UF con rollback\n";
    correr("operaciones aleatorias con checkpoints", pruebaRollbackAleatorio);
    correr("Rollback restaura parent y rank exactos", pruebaRollbackEstadoExacto);
    correr("altura <= log2(n) sin path compression", pruebaRollbackAltura);
    correr("casos borde y checkpoints anidados", pruebaRollbackBordes);
    correr("backtracking del paper (altura 4, p=5/10/15%)", pruebaRollbackBacktrackingPaper);

    std::cout << "\n" << checks - fallos << "/" << checks << " checks OK\n";
    return fallos == 0 ? 0 : 1;
}

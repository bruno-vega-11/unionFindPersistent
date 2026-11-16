//
// Benchmarks de las 5 variantes
//
// Experimento 1 (replica del paper de Conchon y Filliatre):
//   arbol binario completo de altura 4; entre cada nodo y sus hijos se hacen N operaciones
//   sobre n = N elementos, con una proporcion p de uniones (el resto son consultas
//   "x e y estan en el mismo set?", es decir dos Find). Al volver de un hijo se retrocede.
//   N en {2e4, 1e5, 5e5}, p en {5, 10, 15}%.
//
// Experimento 2 (adversarial): se alternan consultas entre dos versiones distantes,
//   cada una a K uniones de una version base comun. Para Baker cada cambio de version
//   cuesta un reroot de O(K); para el segment tree cada acceso cuesta O(log n).
//
// Se mide tiempo (promedio sobre varias semillas) y memoria pico del heap.
// Todas las variantes procesan exactamente la misma secuencia de operaciones; el
// "chequeo" (cantidad de consultas que dieron "mismo set") debe coincidir entre ellas.
//
// Uso:  ./benchmarks          (completo, escribe resultados_paper.csv y resultados_adversarial.csv)
//       ./benchmarks rapido   (configuracion reducida para probar)
//

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "UnionFindEfimero.h"
#include "UnionFindBaker.h"
#include "PersistentArraySegTree.h"
#include "UnionFindSegTree.h"
#include "UnionFindRollback.h"

// ------------------------------------------------------------------ medicion de memoria
// se reemplaza operator new/delete para llevar la cuenta de los bytes vivos en el heap
// (portable: no depende de APIs de Linux ni de Windows)

static size_t bytes_actual = 0;
static size_t bytes_pico = 0;
static constexpr size_t CABECERA = alignof(std::max_align_t);

static void* reservar(size_t n) {
    void* p = std::malloc(n + CABECERA);
    if (!p) return nullptr;
    *static_cast<size_t*>(p) = n;
    bytes_actual += n;
    if (bytes_actual > bytes_pico) bytes_pico = bytes_actual;
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + CABECERA);
}

static void liberar(void* p) noexcept {
    if (!p) return;
    void* real = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) - CABECERA);
    bytes_actual -= *static_cast<size_t*>(real);
    std::free(real);
}

void* operator new(size_t n) {
    void* p = reservar(n);
    if (!p) throw std::bad_alloc();
    return p;
}
void* operator new[](size_t n) { return operator new(n); }
void* operator new(size_t n, const std::nothrow_t&) noexcept { return reservar(n); }
void* operator new[](size_t n, const std::nothrow_t&) noexcept { return reservar(n); }
void operator delete(void* p) noexcept { liberar(p); }
void operator delete[](void* p) noexcept { liberar(p); }
void operator delete(void* p, size_t) noexcept { liberar(p); }
void operator delete[](void* p, size_t) noexcept { liberar(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { liberar(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept { liberar(p); }

struct Medicion {
    double ms = 0;
    double mb = 0;
    long long chequeo = 0;
};

// corre f() midiendo tiempo y memoria pico del heap (por encima de lo que ya estaba reservado)
static Medicion medir(const std::function<long long()>& f) {
    size_t base = bytes_actual;
    bytes_pico = bytes_actual;
    auto t0 = std::chrono::steady_clock::now();
    long long chequeo = f();
    auto t1 = std::chrono::steady_clock::now();
    Medicion m;
    m.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    m.mb = static_cast<double>(bytes_pico - base) / (1024.0 * 1024.0);
    m.chequeo = chequeo;
    return m;
}

// ------------------------------------------------------------------ generador de operaciones
// splitmix64: barato, para que generar las operaciones no pese en la medicion

struct Generador {
    uint64_t s;
    explicit Generador(uint64_t semilla) : s(semilla) {}
    uint64_t siguiente() {
        uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    int menor(int n) { return static_cast<int>(siguiente() % static_cast<uint64_t>(n)); }
};

struct Params {
    int n;       // elementos
    int N;       // operaciones por arista
    int p;       // % de uniones
    int altura;  // altura del arbol de backtracking
};

// ------------------------------------------------------------------ experimento 1: backtracking
// en todas las variantes las llamadas al generador van en el mismo orden (x, y, tipo)

// variantes persistentes (2 y 3): la version del padre se conserva sola
template <typename UF>
static long long explorarPersistente(const UF& v, int prof, const Params& P, Generador& g) {
    if (prof == P.altura) return 0;
    long long iguales = 0;
    for (int hijo = 0; hijo < 2; hijo++) {
        UF w = v;
        for (int k = 0; k < P.N; k++) {
            int x = g.menor(P.n), y = g.menor(P.n);
            if (g.menor(100) < P.p) w = w.Union(x, y);
            else iguales += (w.Find(x) == w.Find(y));
        }
        iguales += explorarPersistente(w, prof + 1, P, g);
    }
    return iguales;
}

// variante 4: igual, pero sin GC hay que liberar el pool del segment tree al retroceder
// (patron seguro: guardar la version antes de la marca y restaurarla tras liberar)
static long long explorarSegTree(UnionFindSegTree v, int prof, const Params& P, Generador& g) {
    if (prof == P.altura) return 0;
    long long iguales = 0;
    for (int hijo = 0; hijo < 2; hijo++) {
        UnionFindSegTree guardada = v;
        size_t marca = PersistentArraySegTree::Marca();
        UnionFindSegTree w = v;
        for (int k = 0; k < P.N; k++) {
            int x = g.menor(P.n), y = g.menor(P.n);
            if (g.menor(100) < P.p) w = w.Union(x, y);
            else iguales += (w.Find(x) == w.Find(y));
        }
        iguales += explorarSegTree(w, prof + 1, P, g);
        PersistentArraySegTree::Liberar(marca);
        v = guardada;
    }
    return iguales;
}

// variante 1: efimero, retrocede guardando una copia completa en cada nodo (O(n) por nodo)
static long long explorarEfimero(UnionFindEfimero& uf, int prof, const Params& P, Generador& g) {
    if (prof == P.altura) return 0;
    long long iguales = 0;
    UnionFindEfimero guardada = uf;
    for (int hijo = 0; hijo < 2; hijo++) {
        for (int k = 0; k < P.N; k++) {
            int x = g.menor(P.n), y = g.menor(P.n);
            if (g.menor(100) < P.p) uf.Union(x, y);
            else iguales += (uf.Find(x) == uf.Find(y));
        }
        iguales += explorarEfimero(uf, prof + 1, P, g);
        uf = guardada;
    }
    return iguales;
}

// variante 5: rollback, retrocede deshaciendo las uniones
static long long explorarRollback(UnionFindRollback& uf, int prof, const Params& P, Generador& g) {
    if (prof == P.altura) return 0;
    long long iguales = 0;
    int cp = uf.Checkpoint();
    for (int hijo = 0; hijo < 2; hijo++) {
        for (int k = 0; k < P.N; k++) {
            int x = g.menor(P.n), y = g.menor(P.n);
            if (g.menor(100) < P.p) uf.Union(x, y);
            else iguales += (uf.Find(x) == uf.Find(y));
        }
        iguales += explorarRollback(uf, prof + 1, P, g);
        uf.Rollback(cp);
    }
    return iguales;
}

static const std::vector<std::string> VARIANTES = {
    "1 efimero (copia)", "2 baker", "3 baker opt (a)", "4 segment tree", "5 rollback"};

static Medicion correrPaper(int variante, const Params& P, uint64_t semilla) {
    Generador g(semilla);
    switch (variante) {
        case 0:
            return medir([&] {
                UnionFindEfimero uf{std::vector<int>(P.n)};
                return explorarEfimero(uf, 0, P, g);
            });
        case 1:
            return medir([&] { return explorarPersistente(UnionFindBaker<>(P.n), 0, P, g); });
        case 2:
            return medir([&] { return explorarPersistente(UnionFindBaker<PersistentArrayBakerOpt>(P.n), 0, P, g); });
        case 3: {
            Medicion m = medir([&] { return explorarSegTree(UnionFindSegTree(P.n), 0, P, g); });
            PersistentArraySegTree::LiberarTodo();
            return m;
        }
        default:
            return medir([&] {
                UnionFindRollback uf(P.n);
                return explorarRollback(uf, 0, P, g);
            });
    }
}

// ------------------------------------------------------------------ experimento 2: adversarial
// dos versiones A y B, cada una a K uniones de la base; se alternan M consultas A, B, A, B, ...

struct ParamsAdv {
    int n;
    int K;
    int M;
};

template <typename UF>
static long long alternarPersistente(const ParamsAdv& P, Generador& g) {
    UF base(P.n);
    UF a = base, b = base;
    for (int k = 0; k < P.K; k++) { int x = g.menor(P.n), y = g.menor(P.n); a = a.Union(x, y); }
    for (int k = 0; k < P.K; k++) { int x = g.menor(P.n), y = g.menor(P.n); b = b.Union(x, y); }
    long long iguales = 0;
    for (int m = 0; m < P.M; m++) {
        const UF& v = (m % 2 == 0) ? a : b;
        int x = g.menor(P.n), y = g.menor(P.n);
        iguales += (v.Find(x) == v.Find(y));
    }
    return iguales;
}

// el rollback no es persistente: para cambiar de version deshace las K uniones y rehace las otras K
static long long alternarRollback(const ParamsAdv& P, Generador& g) {
    UnionFindRollback uf(P.n);
    std::vector<std::pair<int, int>> ua, ub;
    for (int k = 0; k < P.K; k++) { int x = g.menor(P.n), y = g.menor(P.n); ua.emplace_back(x, y); }
    for (int k = 0; k < P.K; k++) { int x = g.menor(P.n), y = g.menor(P.n); ub.emplace_back(x, y); }
    int base = uf.Checkpoint();
    long long iguales = 0;
    for (int m = 0; m < P.M; m++) {
        uf.Rollback(base);
        for (auto [x, y] : (m % 2 == 0) ? ua : ub) uf.Union(x, y);
        int x = g.menor(P.n), y = g.menor(P.n);
        iguales += (uf.Find(x) == uf.Find(y));
    }
    return iguales;
}

static const std::vector<std::string> VARIANTES_ADV = {
    "2 baker", "3 baker opt (a)", "4 segment tree", "5 rollback (rehacer)"};

static Medicion correrAdversarial(int variante, const ParamsAdv& P, uint64_t semilla) {
    Generador g(semilla);
    switch (variante) {
        case 0: return medir([&] { return alternarPersistente<UnionFindBaker<>>(P, g); });
        case 1: return medir([&] { return alternarPersistente<UnionFindBaker<PersistentArrayBakerOpt>>(P, g); });
        case 2: {
            Medicion m = medir([&] { return alternarPersistente<UnionFindSegTree>(P, g); });
            PersistentArraySegTree::LiberarTodo();
            return m;
        }
        default: return medir([&] { return alternarRollback(P, g); });
    }
}

// ------------------------------------------------------------------ main

int main(int argc, char** argv) {
    bool rapido = argc > 1 && std::string(argv[1]) == "rapido";
    std::vector<int> valoresN = rapido ? std::vector<int>{20000} : std::vector<int>{20000, 100000, 500000};
    std::vector<int> valoresP = {5, 10, 15};
    std::vector<int> valoresK = rapido ? std::vector<int>{10, 1000} : std::vector<int>{10, 100, 1000, 10000};
    int semillas = rapido ? 1 : 3;
    bool ok = true;

    std::cout << std::fixed << std::setprecision(1);

    // ---- experimento 1
    std::ofstream csv1("resultados_paper.csv");
    csv1 << "N,p,variante,tiempo_ms,memoria_mb,razon_vs_efimero\n";
    std::cout << "=== Experimento 1: backtracking del paper (altura 4, n = N, " << semillas << " semilla(s))\n";
    for (int N : valoresN) {
        for (int p : valoresP) {
            Params P{N, N, p, 4};
            std::cout << "\nN=" << N << "  p=" << p << "%\n";
            std::cout << std::left << std::setw(22) << "variante" << std::right << std::setw(12) << "tiempo(ms)"
                      << std::setw(12) << "memoria(MB)" << std::setw(12) << "vs efimero" << "\n";
            double ms_efimero = 0;
            long long chequeo_ref = -1;
            for (int v = 0; v < static_cast<int>(VARIANTES.size()); v++) {
                double ms = 0, mb = 0;
                for (int s = 0; s < semillas; s++) {
                    Medicion m = correrPaper(v, P, 1000 + s);
                    ms += m.ms / semillas;
                    mb += m.mb / semillas;
                    long long chequeo_s = m.chequeo;
                    if (v == 0 && s == 0) chequeo_ref = chequeo_s;
                    if (s == 0 && chequeo_s != chequeo_ref) {
                        ok = false;
                        std::cout << "  ERROR: " << VARIANTES[v] << " da un resultado distinto al efimero\n";
                    }
                }
                if (v == 0) ms_efimero = ms;
                double razon = ms / ms_efimero;
                std::cout << std::left << std::setw(22) << VARIANTES[v] << std::right << std::setw(12) << ms
                          << std::setw(12) << mb << std::setw(11) << std::setprecision(2) << razon << "x"
                          << std::setprecision(1) << "\n";
                csv1 << N << "," << p << "," << VARIANTES[v] << "," << ms << "," << mb << "," << razon << "\n";
            }
        }
    }

    // ---- experimento 2
    std::ofstream csv2("resultados_adversarial.csv");
    csv2 << "K,variante,tiempo_ms,memoria_mb,us_por_consulta\n";
    const int n_adv = 100000, M = 2000;
    std::cout << "\n=== Experimento 2: alternar entre dos versiones a distancia K (n=" << n_adv << ", " << M
              << " consultas alternadas)\n";
    for (int K : valoresK) {
        ParamsAdv P{n_adv, K, M};
        std::cout << "\nK=" << K << "\n";
        std::cout << std::left << std::setw(22) << "variante" << std::right << std::setw(12) << "tiempo(ms)"
                  << std::setw(12) << "memoria(MB)" << std::setw(14) << "us/consulta" << "\n";
        long long chequeo_ref = -1;
        for (int v = 0; v < static_cast<int>(VARIANTES_ADV.size()); v++) {
            double ms = 0, mb = 0;
            for (int s = 0; s < semillas; s++) {
                Medicion m = correrAdversarial(v, P, 2000 + s);
                ms += m.ms / semillas;
                mb += m.mb / semillas;
                if (v == 0 && s == 0) chequeo_ref = m.chequeo;
                if (s == 0 && m.chequeo != chequeo_ref) {
                    ok = false;
                    std::cout << "  ERROR: " << VARIANTES_ADV[v] << " da un resultado distinto a baker\n";
                }
            }
            double us = ms * 1000.0 / M;
            std::cout << std::left << std::setw(22) << VARIANTES_ADV[v] << std::right << std::setw(12) << ms
                      << std::setw(12) << mb << std::setw(14) << us << "\n";
            csv2 << K << "," << VARIANTES_ADV[v] << "," << ms << "," << mb << "," << us << "\n";
        }
    }

    std::cout << "\n" << (ok ? "Todas las variantes dieron los mismos resultados." : "HUBO DIFERENCIAS ENTRE VARIANTES")
              << "\nResultados guardados en resultados_paper.csv y resultados_adversarial.csv\n";
    return ok ? 0 : 1;
}

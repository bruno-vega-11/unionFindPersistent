cmake_minimum_required(VERSION 3.31)
project(unionFindPersistent)

set(CMAKE_CXX_STANDARD 20)

add_executable(unionFindPersistent main.cpp
UnionFindEfimero.cpp
UnionFindBaker.cpp
PersistentArraySegTree.cpp
UnionFindSegTree.cpp
UnionFindRollback.cpp)

add_executable(tests tests.cpp
UnionFindEfimero.cpp
UnionFindBaker.cpp
PersistentArraySegTree.cpp
UnionFindSegTree.cpp
UnionFindRollback.cpp)

# benchmarks: siempre optimizado (en Debug los tiempos no sirven)
add_executable(benchmarks benchmarks.cpp
UnionFindEfimero.cpp
UnionFindBaker.cpp
PersistentArraySegTree.cpp
UnionFindSegTree.cpp
UnionFindRollback.cpp)
target_compile_options(benchmarks PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,/O2,-O2>)

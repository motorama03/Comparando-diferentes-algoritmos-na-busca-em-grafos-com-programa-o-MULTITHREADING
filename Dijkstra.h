#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "GrafoBusca.h"
#include <vector>
#include <atomic>

class Dijkstra {
public:
    static bool Sequencial(const GrafoBusca& grafoBusca);
    static bool Paralelo(const GrafoBusca& grafoBusca, int nThreads);
};

#endif
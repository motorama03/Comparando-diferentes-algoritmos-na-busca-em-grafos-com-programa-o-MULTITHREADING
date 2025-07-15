#pragma once
#include "GrafoBusca.h"
#include "json.hpp"
#include "json.hpp"


class FloydWarshall {
public:
    static bool sequencial(const GrafoBusca& grafoBusca);
    static bool paralelo(const GrafoBusca& grafoBusca, int nThreads);
};

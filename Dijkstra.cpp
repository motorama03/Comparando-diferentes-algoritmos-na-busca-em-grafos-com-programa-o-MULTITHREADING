#include "Dijkstra.h"
#include <omp.h>
#include <algorithm>
#include <queue>
#include <fstream>
#include <climits>
#include <vector>
#include <algorithm>
#include "json.hpp"

bool Dijkstra::Sequencial(const GrafoBusca& grafoBusca) {
    const Grafo& grafo = grafoBusca.getGrafo();
    int origem = grafoBusca.getInicio();
    int valorBusca = grafoBusca.getValorBusca();
    int n = grafo.size();

    std::vector<int> dist(n, INT_MAX);
    std::vector<bool> visitado(n, false);
    std::vector<int> predecessores(n, -1);

    dist[origem] = 0;

    for (int i = 0; i < n; ++i) {
        int u = -1;
        for (int j = 0; j < n; ++j) {
            if (!visitado[j] && (u == -1 || dist[j] < dist[u])) {
                u = j;
            }
        }

        if (u == -1) break;

        visitado[u] = true;

        if (grafo.getValorVertice(u) == valorBusca) {
            std::vector<int> caminho;
            for (int v = u; v != -1; v = predecessores[v])
                caminho.push_back(v);
            std::reverse(caminho.begin(), caminho.end());

            nlohmann::json j = caminho;
            std::ofstream file("caminho.json");
            file << j.dump(4);
            file.close();

            return true;
        }

        for (int v : grafo.getVerticesAdjacentes(u)) {
            int peso = 1;
            if (dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;
                predecessores[v] = u;
            }
        }
    }

    nlohmann::json j = std::vector<int>();
    std::ofstream file("caminho.json");
    file << j.dump(4);
    file.close();

    return false;
}

bool Dijkstra::Paralelo(const GrafoBusca& grafoBusca, int nThreads) {
    const Grafo& grafo = grafoBusca.getGrafo();
    int origem = grafoBusca.getInicio();
    int valorBusca = grafoBusca.getValorBusca();
    int n = grafo.size();

    std::vector<int> dist(n, INT_MAX);
    std::vector<bool> visitado(n, false);
    std::vector<int> predecessores(n, -1);

    dist[origem] = 0;
    bool encontrado = false;
    int destinoEncontrado = -1;

    for (int count = 0; count < n && !encontrado; ++count) {
        int u = -1;
        int menorDist = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (!visitado[i] && dist[i] < menorDist) {
                menorDist = dist[i];
                u = i;
            }
        }

        if (u == -1) break;

        visitado[u] = true;

        if (grafo.getValorVertice(u) == valorBusca) {
            destinoEncontrado = u;
            encontrado = true;
            break;
        }

        const std::vector<int>& adj = grafo.getVerticesAdjacentes(u);

        #pragma omp parallel for num_threads(nThreads) shared(dist, visitado, predecessores)
        for (int i = 0; i < adj.size(); ++i) {
            int v = adj[i];
            int peso = 1;

            if (!visitado[v]) {
                int novaDist = dist[u] + peso;

                #pragma omp critical
                {
                    if (novaDist < dist[v]) {
                        dist[v] = novaDist;
                        predecessores[v] = u;

                        if (grafo.getValorVertice(v) == valorBusca) {
                            destinoEncontrado = v;
                            encontrado = true;
                        }
                    }
                }
            }
        }

        #pragma omp flush(encontrado)
        if (encontrado) break;
    }

    std::vector<int> caminho;
    if (destinoEncontrado != -1) {
        for (int v = destinoEncontrado; v != -1; v = predecessores[v])
            caminho.push_back(v);
        std::reverse(caminho.begin(), caminho.end());
    }

    nlohmann::json j = caminho;
    std::ofstream file("caminho.json");
    file << j.dump(4);
    file.close();

    return encontrado;
}
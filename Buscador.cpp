#include "Buscador.h"
#include <omp.h>
#include <algorithm>
#include <queue>
#include <fstream>
#include "json.hpp"

bool Buscador::sequencial(const GrafoBusca& grafoBusca) {
    int n = grafoBusca.getGrafo().size();
    std::vector<bool> visitados(n, false);
    std::vector<int> predecessores(n, -1);
    std::vector<int> visitar = {grafoBusca.getInicio()};

    while (!visitar.empty()) {
        std::vector<int> proximosVisitar;

        for (int verticeVisitando : visitar) {
            if (grafoBusca.getGrafo().getValorVertice(verticeVisitando) == grafoBusca.getValorBusca()) {
                // Reconstruir caminho
                std::vector<int> caminho;
                for (int v = verticeVisitando; v != -1; v = predecessores[v])
                    caminho.push_back(v);
                std::reverse(caminho.begin(), caminho.end());

                nlohmann::json j = caminho;
                std::ofstream file("caminho.json");
                file << j.dump(4);
                file.close();
                return true;
            }

            visitados[verticeVisitando] = true;

            for (int adj : grafoBusca.getGrafo().getVerticesAdjacentes(verticeVisitando)) {
                if (!visitados[adj] && predecessores[adj] == -1) {
                    predecessores[adj] = verticeVisitando;
                    proximosVisitar.push_back(adj);
                }
            }
        }

        visitar = proximosVisitar;
    }

    nlohmann::json j = std::vector<int>();
    std::ofstream file("caminho.json");
    file << j.dump(4);
    file.close();

    return false;
}

#include <fstream>
#include "json.hpp"

bool Buscador::paralelo(const GrafoBusca& grafoBusca, int nThreads) {
    int n = grafoBusca.getGrafo().size();
    std::vector<bool> visitados(n, false);
    std::vector<int> predecessores(n, -1);
    std::vector<int> visitar = {grafoBusca.getInicio()};
    bool encontrado = false;
    int destino = -1;

    while (!visitar.empty() && !encontrado) {
        std::vector<int> proximosVisitar;

        #pragma omp parallel num_threads(nThreads)
        {
            std::vector<int> locaisProximos;
            std::vector<std::pair<int, int>> locaisPredecessores;

            #pragma omp for nowait
            for (int i = 0; i < visitar.size(); ++i) {
                int atual = visitar[i];

                if (grafoBusca.getGrafo().getValorVertice(atual) == grafoBusca.getValorBusca()) {
                    #pragma omp critical
                    {
                        encontrado = true;
                        destino = atual;
                    }
                }

                #pragma omp critical
                {
                    visitados[atual] = true;
                }

                for (int adj : grafoBusca.getGrafo().getVerticesAdjacentes(atual)) {
                    if (!visitados[adj]) {
                        locaisPredecessores.emplace_back(adj, atual);
                        locaisProximos.push_back(adj);
                    }
                }
            }

            #pragma omp critical
            {
                for (auto& [v, p] : locaisPredecessores) {
                    if (predecessores[v] == -1) {
                        predecessores[v] = p;
                    }
                }
                proximosVisitar.insert(proximosVisitar.end(), locaisProximos.begin(), locaisProximos.end());
            }
        }

        #pragma omp flush(encontrado)
        if (encontrado) break;

        std::sort(proximosVisitar.begin(), proximosVisitar.end());
        proximosVisitar.erase(std::unique(proximosVisitar.begin(), proximosVisitar.end()), proximosVisitar.end());

        visitar = proximosVisitar;
    }

    std::vector<int> caminho;
    if (destino != -1) {
        for (int v = destino; v != -1; v = predecessores[v])
            caminho.push_back(v);
        std::reverse(caminho.begin(), caminho.end());
    }

    nlohmann::json j = caminho;
    std::ofstream file("caminho.json");
    file << j.dump(4);
    file.close();

    return encontrado;
}
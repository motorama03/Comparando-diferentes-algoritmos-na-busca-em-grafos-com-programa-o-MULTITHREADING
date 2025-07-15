#include "FloydWarshall.h"
#include <omp.h>
#include <vector>
#include "json.hpp"
#include <fstream>

bool FloydWarshall::sequencial(const GrafoBusca& grafoBusca) {
    int n = grafoBusca.getGrafo().size();
    int inicio = grafoBusca.getInicio();
    int valorBusca = grafoBusca.getValorBusca();

    std::vector<std::vector<bool>> alcancavel(n, std::vector<bool>(n, false));
    std::vector<std::vector<int>> predecessores(n, std::vector<int>(n, -1));

    for (int i = 0; i < n; ++i) {
        alcancavel[i][i] = true;
        predecessores[i][i] = i;
    }

    for (int i = 0; i < n; ++i) {
        for (int adj : grafoBusca.getGrafo().getVerticesAdjacentes(i)) {
            alcancavel[i][adj] = true;
            predecessores[i][adj] = i;
        }
    }

    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (alcancavel[i][k] && alcancavel[k][j]) {
                    if (!alcancavel[i][j]) {
                        alcancavel[i][j] = true;
                        predecessores[i][j] = predecessores[k][j];
                    }
                }

    for (int j = 0; j < n; ++j) {
        if (alcancavel[inicio][j] &&
            grafoBusca.getGrafo().getValorVertice(j) == valorBusca) {

            std::vector<int> caminho;
            int atual = j;
            while (atual != inicio) {
                caminho.push_back(atual);
                atual = predecessores[inicio][atual];
                if (atual == -1) break;
            }
            caminho.push_back(inicio);
            std::reverse(caminho.begin(), caminho.end());

            nlohmann::json jpath = caminho;
            std::ofstream file("caminho.json");
            file << jpath.dump(4);
            file.close();

            return true;
        }
    }

    nlohmann::json jvazio = std::vector<int>();
    std::ofstream file("caminho.json");
    file << jvazio.dump(4);
    file.close();

    return false;
}

bool FloydWarshall::paralelo(const GrafoBusca& grafoBusca, int nThreads) {
    int n = grafoBusca.getGrafo().size();
    int inicio = grafoBusca.getInicio();
    int valorBusca = grafoBusca.getValorBusca();

    std::vector<std::vector<bool>> alcancavel(n, std::vector<bool>(n, false));
    std::vector<std::vector<int>> predecessores(n, std::vector<int>(n, -1));

    #pragma omp parallel for num_threads(nThreads)
    for (int i = 0; i < n; ++i) {
        alcancavel[i][i] = true;
        predecessores[i][i] = i;

        for (int adj : grafoBusca.getGrafo().getVerticesAdjacentes(i)) {
            alcancavel[i][adj] = true;
            predecessores[i][adj] = i;
        }
    }

    for (int k = 0; k < n; ++k) {
        #pragma omp parallel for collapse(2) num_threads(nThreads)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (alcancavel[i][k] && alcancavel[k][j]) {
                    if (!alcancavel[i][j]) {
                        alcancavel[i][j] = true;
                        predecessores[i][j] = predecessores[k][j];
                    }
                }
    }

    int destino = -1;

    #pragma omp parallel for num_threads(nThreads)
    for (int j = 0; j < n; ++j) {
        if (alcancavel[inicio][j] &&
            grafoBusca.getGrafo().getValorVertice(j) == valorBusca) {
            #pragma omp critical
            destino = j;
        }
    }

    std::vector<int> caminho;
    if (destino != -1) {
        int atual = destino;
        while (atual != inicio) {
            caminho.push_back(atual);
            atual = predecessores[inicio][atual];
            if (atual == -1) break;
        }
        caminho.push_back(inicio);
        std::reverse(caminho.begin(), caminho.end());
    }

    nlohmann::json jpath = caminho;
    std::ofstream file("caminho.json");
    file << jpath.dump(4);
    file.close();

    return destino != -1;
}
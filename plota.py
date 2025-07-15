import json
import networkx as nx
import matplotlib.pyplot as plt

with open(r"C:\Users\mateu\OneDrive\Documentos\BCC\BCC-AltoDesempenho\C++\cpp\caminho.json", "r", encoding="utf-8") as f:
    caminho = json.load(f)
    
with open(r"C:\Users\mateu\OneDrive\Documentos\BCC\BCC-AltoDesempenho\C++\cpp\grafos\grafo_500_nodos.json", "r", encoding="utf-8") as f:
    dados = json.load(f)

vertices = dados["grafo"]["vertices"]
adjacencias = dados["grafo"]["adjacencias"]

G = nx.Graph()

for i, vizinhos in enumerate(adjacencias):
    origem = i 
    for destino in vizinhos:
        G.add_edge(origem, destino)

pos = nx.spring_layout(G, seed=42)

nx.draw_networkx_nodes(G, pos, node_color='gray', node_size=20)
nx.draw_networkx_edges(G, pos, edge_color='lightgray')

caminho_arestas = list(zip(caminho, caminho[1:]))
nx.draw_networkx_nodes(G, pos, nodelist=caminho, node_color='red', node_size=50)
nx.draw_networkx_edges(G, pos, edgelist=caminho_arestas, edge_color='red', width=1)

nx.draw_networkx_labels(G, pos, labels={n: str(vertices[n]) for n in caminho}, font_size=8, font_color='black')

plt.title("Figura 03: Caminho encontrado pelo algoritmo de Floyd-Warshall")
plt.axis('off')
plt.tight_layout()
plt.show()
plt.savefig("Busca_em_profundidade_caminho.png", dpi=300)

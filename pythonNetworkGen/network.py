import networkx as nx
import matplotlib.pyplot as plt

G = nx.complete_graph(100)
print(G.nodes()) # returns a list
print(G.edges()) # returns a list

nx.draw(G)
plt.show()




def OutputGraphAsSimConsumable():
    pass


def OutputGraphAsCSVForD3JS():
    pass
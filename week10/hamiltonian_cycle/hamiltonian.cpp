#include <iostream>
#include <map>
#include <vector>

bool isNotAlreadyInPath(int node, const std::vector<int>& path) {
    if(std::find(path.begin(), path.end(), node) != path.end())
        return false;
    return true;
}

bool hasVertexToNode(int node, int destination, const std::map<int, std::vector<int>>& graph) {
    if(std::find(graph.at(node).begin(), graph.at(node).end(), destination) != graph.at(node).end())
        return false;
    return true;
}

bool hamiltonian_cycle(const std::map<int, std::vector<int>>& graph, std::vector<int>& path) {
    int prev_node;
    if(path.size() == graph.size())
        return hasVertexToNode(path[path.size()-1], path[0], graph);
    else {
        prev_node = path[path.size()-1];
        for (auto& next_node: graph.at(prev_node)) {
            if(isNotAlreadyInPath(next_node, path)) {
                path.push_back(next_node);
                if(hamiltonian_cycle(graph, path))
                    return true;
                path.pop_back();
            }
        }
    }
    return false;
}
int main() {
    std::vector<int> path = {6};
    std::map<int, std::vector<int>> test_graph_0 = {
            {1, {2, 5}},
            {2, {1, 3}},
            {3, {2, 6}},
            {4, {3, 5}},
            {5, {1, 4}},
            {6, {1, 2, 3}}
    };
    std::map<int, std::vector<int>> test_graph_1 = {
            {1, {2, 3}},
            {2, {3}},
            {3, {}}
    };
    if(hamiltonian_cycle(test_graph_0, path)) {
        for(auto value: path) {
            std::cout << value << " ";
        }
    }
    return 0;
}

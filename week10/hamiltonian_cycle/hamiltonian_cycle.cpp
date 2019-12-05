#include <iostream>
#include <map>
#include <vector>
#include <future>

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

bool good(std::vector<int>* candidate_solution) {
    for(size_t i=0; i < candidate_solution->size()-1; i++) {
        for(size_t j=i+1; j < candidate_solution->size(); j++) {
            if((*candidate_solution)[i] == (*candidate_solution)[j]) {
                return false;
            }
        }
    }
    return true;
}

void isHamiltonianCycle(std::vector<int>* last_candidate, unsigned int candidate_pos, const std::map<int, std::vector<int>>* graph, std::vector<bool>* solutions) {
    for(int i=0; i < last_candidate->size() - 1; i++) {
        if(std::find(graph->at((*last_candidate)[i]).begin(), graph->at((*last_candidate)[i]).end(), (*last_candidate)[i+1]) == graph->at((*last_candidate)[i]).end()) {
            (*solutions)[candidate_pos] = false;
            return;
        }
    }
    (*solutions)[candidate_pos] = true;
}

void hami(std::vector<std::vector<int>*>* collection, std::vector<int>* v, int pos, int graph_size, const std::map<int, std::vector<int>>* graph, std::vector<bool>* solutions) {
    if(pos == graph_size) {
        if(good(v)) {
            // copying the v into a new memory location
            auto nv = new std::vector<int>();
            for (auto value: *v) {
                nv->push_back(value);
            }
            collection->push_back(nv);
            // end copying

            // let's check on a diff thread if it is a hamiltonian cycle
            std::vector<int>* last_candidate = (*collection)[collection->size()-1];
            unsigned int candidate_pos = collection->size()-1;
            std::future<void> ft = std::async(std::launch::async, isHamiltonianCycle, last_candidate, candidate_pos, graph, solutions);
            // end check
        }
    } else {
        for(size_t i=1; i <= graph_size; i++) {
            (*v)[pos] = i;
            hami(collection, v, pos + 1, graph_size, graph, solutions);
        }
        (*v)[pos] = 0;
    }
}

int factorial(int n) {
    return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
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

    // using the test graph 0
    if(hamiltonian_cycle(test_graph_0, path)) {
        for(auto value: path) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }

    // using the test graph 1
    std::vector<std::vector<int>*> collection_test_graph_1;
    std::vector<int> v_test_graph_1{0, 0, 0};
    std::vector<bool> solutions_test_graph_1;
    for(int i=0; i<factorial(test_graph_1.size()); i++) {
        solutions_test_graph_1.push_back(false);
    }

    hami(&collection_test_graph_1, &v_test_graph_1, 0, test_graph_1.size(), &test_graph_1, &solutions_test_graph_1);
    for (size_t j=0; j < collection_test_graph_1.size(); j++) {
        if(solutions_test_graph_1[j]) {
            for (size_t i = 0; i < (*collection_test_graph_1[j]).size(); i++) {
                std::cout << (*collection_test_graph_1[j])[i] << " ";
            }
            std::cout << std::endl;
        }
    }

    // using the test graph 0
    std::vector<std::vector<int>*> collection_test_graph_0;
    std::vector<int> v_test_graph_0{0, 0, 0, 0, 0, 0};
    std::vector<bool> solutions_test_graph_0;
    for(int i=0; i<factorial(test_graph_0.size()); i++) {
        solutions_test_graph_0.push_back(false);
    }

    hami(&collection_test_graph_0, &v_test_graph_0, 0, test_graph_0.size(), &test_graph_0, &solutions_test_graph_0);
    for (size_t j=0; j < collection_test_graph_0.size(); j++) {
        if(solutions_test_graph_0[j]) {
            for (size_t i = 0; i < (*collection_test_graph_0[j]).size(); i++) {
                std::cout << (*collection_test_graph_0[j])[i] << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}

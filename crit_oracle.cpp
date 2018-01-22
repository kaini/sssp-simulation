#include "crit_oracle.hpp"
#include "crit_dijkstra.hpp"
#include <cfloat>

sssp::oracle::oracle(const sssp::graph* graph, size_t start_node) : criteria(graph, start_node) {
    boost::base_collection<criteria> cs;
    cs.insert(smallest_tentative_distance(graph, start_node));
    m_result = dijkstra(*graph, start_node, cs);
}

void sssp::oracle::relaxable_nodes(std::unordered_set<size_t>& output) const {
    for (auto node : m_fringe) {
        if (std::abs(m_result[node.first].distance - node.second) <= DBL_EPSILON) {
            output.insert(node.first);
        }
    }
}

void sssp::oracle::changed_predecessor(size_t node, size_t predecessor, double distance) {
    m_fringe[node] = distance;
}

void sssp::oracle::relaxed_node(size_t node) {
    m_fringe.erase(node);
}

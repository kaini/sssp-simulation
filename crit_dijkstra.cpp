#include "crit_dijkstra.hpp"

sssp::smallest_tentative_distance::smallest_tentative_distance(const sssp::graph* graph, size_t start_node)
    : criteria(graph, start_node), m_node_info(graph->make_node_map([](size_t i) { return node_info(i); })) {}

void sssp::smallest_tentative_distance::relaxable_nodes(std::unordered_set<size_t>& output) const {
    if (!m_queue.empty()) {
        output.insert(m_queue.top()->index);
    }
}

void sssp::smallest_tentative_distance::changed_predecessor(size_t node, size_t predecessor, double distance) {
    m_node_info[node].tentative_distance = distance;
    if (m_node_info[node].queue_handle == queue::handle_type()) {
        m_node_info[node].queue_handle = m_queue.push(&m_node_info[node]);
    } else {
        m_queue.update(m_node_info[node].queue_handle);
    }
}

void sssp::smallest_tentative_distance::relaxed_node(size_t node) {
    m_queue.erase(m_node_info[node].queue_handle);
}

bool sssp::smallest_tentative_distance::node_info_compare::operator()(const node_info* a, const node_info* b) const {
    return a->tentative_distance > b->tentative_distance;
}

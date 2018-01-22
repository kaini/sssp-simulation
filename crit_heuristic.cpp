#include "crit_heuristic.hpp"
#include <boost/assert.hpp>
#include <boost/heap/pairing_heap.hpp>
#include <cfloat>
#include <unordered_set>

sssp::heuristic::heuristic(const sssp::graph* graph, size_t start_node, relaxation_heuristic heuristic)
    : criteria(graph, start_node), m_heuristic(heuristic),
      m_node_info(graph->make_node_map([&](size_t node) { return node_info(*graph, heuristic, node); })) {

    node_info& start = m_node_info[start_node];
    if (start.unsettled_predecessors.empty() ||
        start.tentative_distance <= start.unsettled_predecessors.back().estimated_distance) {
        m_safe_to_relax.insert(start_node);
    }
}

void sssp::heuristic::relaxable_nodes(std::unordered_set<size_t>& output) const {
    output.insert(m_safe_to_relax.begin(), m_safe_to_relax.end());
}

void sssp::heuristic::changed_predecessor(size_t node, size_t predecessor, double distance) {
    BOOST_ASSERT(!m_node_info[node].settled);
    m_node_info[node].tentative_distance = distance;
}

void sssp::heuristic::relaxed_node(size_t node) {
    m_node_info[node].settled = true;

    auto iter = m_safe_to_relax.find(node);
    if (iter != m_safe_to_relax.end()) {
        m_safe_to_relax.erase(iter);
    }

    for (const auto& outgoing_edge : graph().outgoing_edges(node)) {
        node_info& successor = m_node_info[outgoing_edge.destination];
        while (!successor.unsettled_predecessors.empty() &&
               m_node_info[successor.unsettled_predecessors.back().index].settled) {
            successor.unsettled_predecessors.pop_back();
        }
        if (!successor.settled &&
            (successor.unsettled_predecessors.empty() ||
             successor.tentative_distance <= successor.unsettled_predecessors.back().estimated_distance)) {
            m_safe_to_relax.insert(successor.index);
        }
    }
}

sssp::heuristic::node_info::node_info(const sssp::graph& graph, relaxation_heuristic h, size_t index) : index(index) {
    for (const auto& incoming_edge : graph.incoming_edges(index)) {
        unsettled_predecessors.emplace_back(incoming_edge.source, h(incoming_edge.source) + incoming_edge.cost);
    }
    std::sort(unsettled_predecessors.begin(), unsettled_predecessors.end(), [](const auto& a, const auto& b) {
        return a.estimated_distance > b.estimated_distance;
    });
}

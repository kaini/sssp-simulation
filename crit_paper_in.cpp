#include "crit_paper_in.hpp"

sssp::paper_in::paper_in(const sssp::graph* graph, size_t start_node) :
    criteria(graph, start_node),
    m_node_info(graph->node_count())
{}

void sssp::paper_in::relaxable_nodes(todo_output& output) const {
    double min_tent = INFINITY;
    for (size_t node = 0; node < graph().node_count(); ++node) {
        if (!m_node_info[node].settled) {
            min_tent = std::min(min_tent, m_node_info[node].tentative_distance);
        }
    }

    for (size_t node = 0; node < graph().node_count(); ++node) {
        if (m_node_info[node].settled || m_node_info[node].tentative_distance == INFINITY) {
            continue;
        }

        double min_incoming = INFINITY;
        for (edge_info edge : graph().incoming_edges(node)) {
            if (m_node_info[edge.source].settled) {
                // predecessor in S => do nothing
            } else if (m_node_info[edge.source].tentative_distance == INFINITY) {
                // predecessor in U
                for (edge_info next_edge : graph().incoming_edges(edge.source)) {
                    min_incoming = std::min(min_incoming, edge.cost + next_edge.cost);
                }
            } else {
                // predecessor in F
                min_incoming = std::min(min_incoming, edge.cost);
            }
        }

        if (m_node_info[node].tentative_distance - min_incoming <= min_tent) {
            output.emplace(node);
        }
    }
}

void sssp::paper_in::changed_predecessor(size_t node, size_t predecessor, double distance) {
    BOOST_ASSERT(!m_node_info[node].settled);
    m_node_info[node].tentative_distance = distance;
}

void sssp::paper_in::relaxed_node(size_t node) {
    BOOST_ASSERT(!m_node_info[node].settled);
    m_node_info[node].settled = true;
}

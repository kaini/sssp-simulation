#include "crit_paper_out.hpp"

sssp::paper_out::paper_out(const sssp::graph* graph, size_t start_node)
    : criteria(graph, start_node), m_node_info(graph->node_count()) {}

void sssp::paper_out::relaxable_nodes(todo_output& output) const {
    double min_threshold = INFINITY;
    for (size_t node = 0; node < graph().node_count(); ++node) {
        if (m_node_info[node].settled || m_node_info[node].tentative_distance == INFINITY) {
            continue;
        }

        for (edge_info edge : graph().outgoing_edges(node)) {
            if (m_node_info[edge.destination].settled) {
                // successor in S => do nothing
            } else if (m_node_info[edge.destination].tentative_distance == INFINITY) {
                // successor in U
                for (edge_info next_edge : graph().outgoing_edges(edge.destination)) {
                    if (!m_node_info[next_edge.destination].settled) {
                        min_threshold = std::min(min_threshold, m_node_info[node].tentative_distance + edge.cost + next_edge.cost);
                    }
                }
            } else {
                // successor in F
                min_threshold = std::min(min_threshold, m_node_info[node].tentative_distance + edge.cost);
            }
        }
    }

    for (size_t node = 0; node < graph().node_count(); ++node) {
        if (m_node_info[node].settled || m_node_info[node].tentative_distance == INFINITY) {
            continue;
        }
        if (m_node_info[node].tentative_distance <= min_threshold) {
            output.emplace(node);
        }
    }
}

void sssp::paper_out::changed_predecessor(size_t node, size_t predecessor, double distance) {
    BOOST_ASSERT(!m_node_info[node].settled);
    m_node_info[node].tentative_distance = distance;
}

void sssp::paper_out::relaxed_node(size_t node) {
    BOOST_ASSERT(!m_node_info[node].settled);
    m_node_info[node].settled = true;
}

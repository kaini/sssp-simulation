#include "graph.hpp"
#include <algorithm>
#include <boost/assert.hpp>

sssp::graph::graph() {}

size_t sssp::graph::add_node() {
    size_t index = m_nodes.size();
    m_nodes.emplace_back();
    return index;
}

void sssp::graph::add_edge(size_t source, size_t destination, double cost) {
    BOOST_ASSERT(
        std::find_if(m_nodes[source].outgoing.begin(), m_nodes[source].outgoing.end(), [&](const edge_info& e) {
            return e.destination == destination;
        }) == m_nodes[source].outgoing.end());
    BOOST_ASSERT(std::find_if(m_nodes[destination].incoming.begin(),
                              m_nodes[destination].incoming.end(),
                              [&](const edge_info& e) { return e.source == source; }) ==
                 m_nodes[destination].incoming.end());
    BOOST_ASSERT(source != destination);
    BOOST_ASSERT(cost >= 0.0);
    m_nodes[source].outgoing.emplace_back(edge_info(source, destination, cost));
    m_nodes[destination].incoming.emplace_back(edge_info(source, destination, cost));
}

size_t sssp::graph::node_count() const {
    return m_nodes.size();
}

const std::vector<sssp::edge_info>& sssp::graph::outgoing_edges(size_t source) const {
    return m_nodes[source].outgoing;
}

const std::vector<sssp::edge_info>& sssp::graph::incoming_edges(size_t destination) const {
    return m_nodes[destination].incoming;
}

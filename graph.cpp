#include "graph.hpp"

sssp::graph::graph() {
}

size_t sssp::graph::add_node() {
	size_t index = m_nodes.size();
	m_nodes.emplace_back();
	return index;
}

void sssp::graph::add_edge(size_t source, size_t destination, double cost) {
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

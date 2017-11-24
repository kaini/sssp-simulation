#include "graph.hpp"

sssp::graph::graph() {
}

size_t sssp::graph::add_node() {
	size_t index = m_nodes.size();
	m_nodes.emplace_back();
	return index;
}

void sssp::graph::add_edge(size_t source, size_t destination, double cost) {
	m_nodes[source].outgoing.emplace_back(edge{destination, cost});
	m_nodes[destination].incoming.emplace_back(edge{source, cost});
}

size_t sssp::graph::node_count() const {
	return m_nodes.size();
}

std::vector<sssp::edge_info> sssp::graph::outgoing_edges(size_t source) const {
	std::vector<edge_info> result;
	result.reserve(m_nodes[source].outgoing.size());
	for (const edge& edge : m_nodes[source].outgoing) {
		result.emplace_back(edge_info{source, edge.node, edge.cost});
	}
	return result;
}

std::vector<sssp::edge_info> sssp::graph::incoming_edges(size_t destination) const {
	std::vector<edge_info> result;
	result.reserve(m_nodes[destination].incoming.size());
	for (const edge& edge : m_nodes[destination].incoming) {
		result.emplace_back(edge_info{edge.node, destination, edge.cost});
	}
	return result;
}

std::vector<sssp::edge_info> sssp::graph::edges() const
{
	std::vector<edge_info> result;
	for (size_t source = 0; source < m_nodes.size(); ++source) {
		for (const edge& edge : m_nodes[source].outgoing) {
			result.emplace_back(edge_info{source, edge.node, edge.cost});
		}
	}
	return result;
}

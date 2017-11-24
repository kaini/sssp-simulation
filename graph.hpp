#pragma once
#include <optional>
#include <vector>
#include <unordered_map>

namespace sssp {

struct edge_info {
	size_t source = -1;
	size_t destination = -1;
	double cost = 0.0;
};

// A directed graph. This datastructure is designed to be very flexible, but still
// tries to be very performant.
class graph {
public:
	// Empty graph.
	graph();

	// Add a new node without edges.
	size_t add_node();
	// Add an edge between two nodes.
	void add_edge(size_t source, size_t destination, double cost);

	// Returns the number of nodes.
	size_t node_count() const;

	// Returns all outgoing edges of a node.
	std::vector<edge_info> outgoing_edges(size_t source) const;
	// Returns all incoming edges of a node.
	std::vector<edge_info> incoming_edges(size_t destination) const;
	// Returns all edges.
	std::vector<edge_info> edges() const;

private:
	struct edge {
		size_t node;
		double cost;
	};

	struct node {
		std::vector<edge> incoming;
		std::vector<edge> outgoing;
	};

	std::vector<node> m_nodes;
};
	
}

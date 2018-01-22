#pragma once
#include <boost/functional/hash.hpp>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace sssp {

struct edge_info {
    edge_info(size_t source, size_t destination, double cost) : source(source), destination(destination), cost(cost) {}
    size_t source;
    size_t destination;
    double cost;
};

template <typename T> using node_map = std::vector<T>;

struct edge_id {
    edge_id(size_t source, size_t destination) : source(source), destination(destination) {}
    size_t source;
    size_t destination;
};

inline bool operator==(const edge_id& a, const edge_id& b) {
    return a.source == b.source && a.destination == b.destination;
}

inline bool operator!=(const edge_id& a, const edge_id& b) {
    return !(a == b);
}

template <typename T> using edge_map = std::unordered_map<edge_id, T>;

// A directed graph. This datastructure is designed to be very flexible, but still
// tries to be very performant.
class graph {
  public:
    // Empty graph.
    graph(size_t expected_nodes = 0, size_t expected_edges_per_node = 0);

    // Add a new node without edges.
    size_t add_node();
    // Add an edge between two nodes.
    void add_edge(size_t source, size_t destination, double cost);

    // Returns the number of nodes.
    size_t node_count() const;

    // Returns all outgoing edges of a node.
    const std::vector<edge_info>& outgoing_edges(size_t source) const;
    // Returns all incoming edges of a node.
    const std::vector<edge_info>& incoming_edges(size_t destination) const;

    // Creates a pre-filled node map by calling the callback.
    template <typename Fun> auto make_node_map(Fun fn) const -> node_map<decltype(fn(0))> {
        node_map<decltype(fn(0))> result;
        result.reserve(m_nodes.size());
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            result.emplace_back(fn(i));
        }
        return result;
    }

    // Creates a pre-filled edge map by calling the callback.
    template <typename Fun> auto make_edge_map(Fun fn) const -> edge_map<decltype(fn(0, 0))> {
        edge_map<decltype(fn(0, 0))> result;
        result.reserve(m_nodes.size() * m_expected_edges);
        for (size_t source = 0; source < m_nodes.size(); ++source) {
            for (const edge_info& edge : m_nodes[source].outgoing) {
                result[{source, edge.destination}] = fn(source, edge.destination);
            }
        }
        return result;
    }

  private:
    struct node {
        std::vector<edge_info> incoming;
        std::vector<edge_info> outgoing;
    };

    std::vector<node> m_nodes;
    size_t m_expected_edges;
};

} // namespace sssp

namespace std {

template <> struct hash<sssp::edge_id> {
    size_t operator()(const sssp::edge_id value) const {
        size_t hash = 0;
        boost::hash_combine(hash, value.source);
        boost::hash_combine(hash, value.destination);
        return hash;
    }
};

} // namespace std

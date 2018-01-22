#include "crit_crauser.hpp"
#include <boost/assert.hpp>

sssp::crauser_in::crauser_in(const sssp::graph* graph, size_t start_node, bool dynamic)
    : criteria(graph, start_node), m_node_info(graph->make_node_map([&](size_t n) { return node_info(*graph, n); })),
      m_dynamic(dynamic) {}

void sssp::crauser_in::relaxable_nodes(todo_output& output) const {
    if (!m_distance_queue.empty()) {
        double m = m_distance_queue.top()->tentative_distance;

        auto iter = m_threshold_queue.ordered_begin();
        auto end = m_threshold_queue.ordered_end();
        while (iter != end && (*iter)->threshold() <= m) {
            output.push_back((*iter)->index);
            ++iter;
        }
    }
}

void sssp::crauser_in::changed_predecessor(size_t node, size_t predecessor, double distance) {
    node_info& info = m_node_info[node];
    info.tentative_distance = distance;
    if (info.distance_queue_handle == distance_queue::handle_type()) {
        info.distance_queue_handle = m_distance_queue.push(&info);
        info.threshold_queue_handle = m_threshold_queue.push(&info);
    } else {
        m_distance_queue.update(info.distance_queue_handle);
        m_threshold_queue.update(info.threshold_queue_handle);
    }
}

void sssp::crauser_in::relaxed_node(size_t node) {
    node_info& info = m_node_info[node];
    info.settled = true;
    m_distance_queue.erase(info.distance_queue_handle);
    m_threshold_queue.erase(info.threshold_queue_handle);

    if (m_dynamic) {
        for (const auto& outgoing : graph().outgoing_edges(node)) {
            node_info& dest = m_node_info[outgoing.destination];
            if (!dest.settled) {
                while (!dest.incoming.empty() && m_node_info[dest.incoming.back().source].settled) {
                    dest.incoming.pop_back();
                }
                if (dest.threshold_queue_handle != threshold_queue::handle_type()) {
                    m_threshold_queue.update(dest.threshold_queue_handle);
                }
            }
        }
    }
}

bool sssp::crauser_in::node_info_compare_distance::operator()(const node_info* a, const node_info* b) const {
    return a->tentative_distance > b->tentative_distance;
}

bool sssp::crauser_in::node_info_compare_threshold::operator()(const node_info* a, const node_info* b) const {
    return a->threshold() > b->threshold();
}

sssp::crauser_in::node_info::node_info(const sssp::graph& g, size_t index)
    : index(index), incoming(g.incoming_edges(index).begin(), g.incoming_edges(index).end()) {
    std::sort(incoming.begin(), incoming.end(), [](const auto& a, const auto& b) { return a.cost > b.cost; });
}

double sssp::crauser_in::node_info::threshold() const {
    if (incoming.empty()) {
        return -INFINITY;
    } else {
        return tentative_distance - incoming.back().cost;
    }
}

sssp::crauser_out::crauser_out(const sssp::graph* graph, size_t start_node, bool dynamic)
    : criteria(graph, start_node), m_node_info(graph->make_node_map([&](size_t n) { return node_info(*graph, n); })),
      m_dynamic(dynamic) {}

void sssp::crauser_out::relaxable_nodes(todo_output& output) const {
    if (!m_distance_queue.empty()) {
        double l = m_threshold_queue.top()->threshold();

        auto iter = m_distance_queue.ordered_begin();
        auto end = m_distance_queue.ordered_end();
        while (iter != end && (*iter)->tentative_distance <= l) {
            output.push_back((*iter)->index);
            ++iter;
        }
    }
}

void sssp::crauser_out::changed_predecessor(size_t node, size_t predecessor, double distance) {
    node_info& info = m_node_info[node];
    info.tentative_distance = distance;
    if (info.distance_queue_handle == distance_queue::handle_type()) {
        info.distance_queue_handle = m_distance_queue.push(&info);
        info.threshold_queue_handle = m_threshold_queue.push(&info);
    } else {
        m_distance_queue.update(info.distance_queue_handle);
        m_threshold_queue.update(info.threshold_queue_handle);
    }
}

void sssp::crauser_out::relaxed_node(size_t node) {
    node_info& info = m_node_info[node];
    info.settled = true;

    m_distance_queue.erase(info.distance_queue_handle);
    m_threshold_queue.erase(info.threshold_queue_handle);
    if (m_dynamic) {
        for (const auto& edge : graph().incoming_edges(node)) {
            node_info& source = m_node_info[edge.source];
            if (!source.settled) {
                while (!source.outgoing.empty() && m_node_info[source.outgoing.back().destination].settled) {
                    source.outgoing.pop_back();
                }
                if (source.threshold_queue_handle != threshold_queue::handle_type()) {
                    m_threshold_queue.update(source.threshold_queue_handle);
                }
            }
        }
    }
}

bool sssp::crauser_out::node_info_compare_distance::operator()(const node_info* a, const node_info* b) const {
    return a->tentative_distance > b->tentative_distance;
}

bool sssp::crauser_out::node_info_compare_threshold::operator()(const node_info* a, const node_info* b) const {
    return a->threshold() > b->threshold();
}

sssp::crauser_out::node_info::node_info(const sssp::graph& g, size_t index)
    : index(index), outgoing(g.outgoing_edges(index).begin(), g.outgoing_edges(index).end()) {
    std::sort(outgoing.begin(), outgoing.end(), [](const auto& a, const auto& b) { return a.cost > b.cost; });
}

double sssp::crauser_out::node_info::threshold() const {
    if (outgoing.empty()) {
        return INFINITY;
    } else {
        return tentative_distance + outgoing.back().cost;
    }
}

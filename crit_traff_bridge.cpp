#include "crit_traff_bridge.hpp"

sssp::traff_bridge::traff_bridge(const sssp::graph* graph, size_t start_node)
    : criteria(graph, start_node),
      m_info(graph->make_node_map([&](size_t n) { return node_info(*graph, n, m_pool); })) {}

void sssp::traff_bridge::relaxable_nodes(todo_output& output) const {
    if (!m_distance_queue.empty()) {
        double t = m_distance_queue.top()->tentative;

        auto iter = m_threshold_queue.ordered_begin();
        auto end = m_threshold_queue.ordered_end();
        while (iter != end && (*iter)->threshold() <= t) {
            output.push_back((*iter)->index);
            ++iter;
        }
    }
}

void sssp::traff_bridge::changed_predecessor(size_t node, size_t predecessor, double distance) {
    node_info& info = m_info[node];
    info.tentative = distance;

    if (info.distance_queue_handle == distance_queue::handle_type()) {
        info.distance_queue_handle = m_distance_queue.push(&info);
        info.threshold_queue_handle = m_threshold_queue.push(&info);
    } else {
        m_distance_queue.update(info.distance_queue_handle);
        m_threshold_queue.update(info.threshold_queue_handle);
    }
}

void sssp::traff_bridge::relaxed_node(size_t node) {
    node_info& info = m_info[node];
    info.settled = true;
    m_distance_queue.erase(info.distance_queue_handle);
    m_threshold_queue.erase(info.threshold_queue_handle);

    for (const auto& outgoing_edge : graph().outgoing_edges(node)) {
        node_info& succ = m_info[outgoing_edge.destination];
        if (!succ.settled) {
            while (!succ.predecessors.empty() && m_info[succ.predecessors.back().pred].settled) {
                succ.predecessors.pop_back();
            }
        }
    }
}

sssp::traff_bridge::node_info::node_info(const sssp::graph& graph,
                                         size_t index,
                                         const local_linear_allocator<pred_info>& pool)
    : index(index), predecessors(pool) {
    for (const auto& incoming_edge : graph.incoming_edges(index)) {
        double cost = INFINITY;
        for (const auto& pred_incoming_edge : graph.incoming_edges(incoming_edge.source)) {
            cost = std::min(cost, pred_incoming_edge.cost);
        }
        cost += incoming_edge.cost;
        predecessors.emplace_back(incoming_edge.source, cost);
    }
    std::sort(predecessors.begin(), predecessors.end(), [](const auto& a, const auto& b) { return a.cost > b.cost; });
}

double sssp::traff_bridge::node_info::threshold() const {
    if (predecessors.empty()) {
        return -INFINITY;
    } else {
        return tentative - predecessors.back().cost;
    }
}

bool sssp::traff_bridge::node_info_compare_distance::operator()(const node_info* a, const node_info* b) const {
    return a->tentative > b->tentative;
}

bool sssp::traff_bridge::node_info_compare_threshold::operator()(const node_info* a, const node_info* b) const {
    return a->threshold() > b->threshold();
}

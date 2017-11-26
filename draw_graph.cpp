#include "draw_graph.hpp"

static void draw_edge(const Cairo::RefPtr<Cairo::Context>& cr,
                      const sssp::node_style& source_style,
                      const sssp::node_style& destination_style,
                      const sssp::edge_style& edge_style) {
    cr->save();
    cr->set_source_rgb(edge_style.color.r, edge_style.color.g, edge_style.color.b);
    cr->set_line_width(edge_style.line_width);
    cr->move_to(source_style.position.x, source_style.position.y);
    cr->line_to(destination_style.position.x, destination_style.position.y);
    cr->stroke();

    double angle = std::atan2(source_style.position.y - destination_style.position.y,
                              source_style.position.x - destination_style.position.x);
    sssp::vec2 arrow_end{
        destination_style.position.x + 0.015 * std::cos(angle),
        destination_style.position.y + 0.015 * std::sin(angle),
    };
    cr->move_to(arrow_end.x, arrow_end.y);
    cr->line_to(arrow_end.x + 0.015 * std::cos(angle + M_PI / 6.0), arrow_end.y + 0.015 * std::sin(angle + M_PI / 6.0));
    cr->line_to(arrow_end.x + 0.015 * std::cos(angle - M_PI / 6.0), arrow_end.y + 0.015 * std::sin(angle - M_PI / 6.0));
    cr->close_path();
    cr->stroke_preserve();
    cr->fill();
    cr->restore();
}

void sssp::draw_graph(const Cairo::RefPtr<Cairo::Context>& cr,
                      const graph& graph,
                      const node_map<node_style>& node_styles,
                      const edge_map<edge_style>& edge_styles) {
    for (bool foreground : {false, true}) {
        for (size_t node = 0; node < graph.node_count(); ++node) {
            for (const sssp::edge_info& edge : graph.outgoing_edges(node)) {
                const node_style& source_style = node_styles[edge.source];
                const node_style& destination_style = node_styles[edge.destination];
                const edge_style& edge_style = edge_styles.at({edge.source, edge.destination});
                if (edge_style.foreground == foreground) {
                    draw_edge(cr, source_style, destination_style, edge_style);
                }
            }
        }
    }

    cr->save();
    cr->set_line_width(0.002);
    for (size_t i = 0; i < graph.node_count(); ++i) {
        const node_style& node_style = node_styles[i];
        cr->arc(node_style.position.x, node_style.position.y, 0.015, 0.0, 2.0 * M_PI);
        cr->set_source_rgb(node_style.color.r, node_style.color.g, node_style.color.b);
        cr->fill_preserve();
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->stroke();
    }
    cr->restore();
}

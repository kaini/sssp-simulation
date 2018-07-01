#include "draw_graph.hpp"

static void draw_edge(const Cairo::RefPtr<Cairo::Context>& cr,
                      const sssp::node_style& source_style,
                      const sssp::node_style& destination_style,
                      const sssp::edge_style& edge_style,
                      bool draw_line) {
    cr->save();

    cr->set_source_rgb(edge_style.color.r, edge_style.color.g, edge_style.color.b);
    cr->set_line_width(edge_style.line_width);
    if (draw_line) {
		cr->move_to(source_style.position.x, source_style.position.y);
		cr->line_to(destination_style.position.x, destination_style.position.y);
        cr->stroke();
    }

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
    std::set<std::pair<size_t, size_t>> drawn_edge_lines;

    for (bool foreground : {false, true}) {
        for (size_t node = 0; node < graph.node_count(); ++node) {
            for (const edge_info& edge : graph.outgoing_edges(node)) {

                const edge_style& edge_style = edge_styles.at({edge.source, edge.destination});
                if (edge_style.foreground == foreground) {
                    const node_style& source_style = node_styles[edge.source];
                    const node_style& destination_style = node_styles[edge.destination];
                    draw_edge(cr,
                              source_style,
                              destination_style,
                              edge_style,
                              drawn_edge_lines.find({edge.source, edge.destination}) == drawn_edge_lines.end());
                    drawn_edge_lines.emplace(edge.source, edge.destination);
                    drawn_edge_lines.emplace(edge.destination, edge.source);
                }
            }
        }
    }

    cr->save();
    cr->set_line_width(0.002);
    cr->set_font_size(0.015);
    for (size_t i = 0; i < graph.node_count(); ++i) {
        const node_style& node_style = node_styles[i];

        cr->arc(node_style.position.x, node_style.position.y, 0.015, 0.0, 2.0 * M_PI);
        cr->set_source_rgb(node_style.color.r, node_style.color.g, node_style.color.b);
        cr->fill_preserve();
        cr->set_source_rgb(0.0, 0.0, 0.0);
        cr->stroke();

        Cairo::TextExtents te;
        cr->get_text_extents(node_style.text, te);
        cr->move_to(node_style.position.x - te.width / 2.0 - te.x_bearing,
                    node_style.position.y - te.height / 2.0 - te.y_bearing);
        cr->show_text(node_style.text);
        cr->begin_new_path();
    }
    cr->restore();

    cr->save();
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->set_font_size(0.0175);
    for (size_t node = 0; node < graph.node_count(); ++node) {
        for (const edge_info& edge : graph.outgoing_edges(node)) {
            const edge_style& edge_style = edge_styles.at({edge.source, edge.destination});
            if (!edge_style.text.empty()) {
                const node_style& source_style = node_styles[edge.source];
                const node_style& destination_style = node_styles[edge.destination];
                Cairo::TextExtents te;
                cr->get_text_extents(edge_style.text, te);
                double angle = std::atan2(destination_style.position.y - source_style.position.y,
                                          destination_style.position.x - source_style.position.x);
                cr->save();
                cr->translate(source_style.position.x * 0.5 + destination_style.position.x * 0.5,
                              source_style.position.y * 0.5 + destination_style.position.y * 0.5);
                cr->rotate(angle);
                cr->move_to(-te.width / 2.0 - te.x_bearing, -0.004);
                cr->show_text(edge_style.text);
                cr->begin_new_path();
                cr->restore();
            }
        }
    }
    cr->restore();
}

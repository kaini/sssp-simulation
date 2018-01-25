#include "run.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef DISABLE_CAIRO
#include "draw_graph.hpp"
#include <cairomm/cairomm.h>
#endif

int main(int argc, char* argv[]) {
    boost::optional<sssp::arguments> args_opt = sssp::parse_arguments(argc, argv, &std::cerr);
    if (!args_opt) {
        return EXIT_FAILURE;
    }
    sssp::arguments args = *args_opt;

    std::cout << "# sssp-simulation";
    for (int i = 1; i < argc; ++i) {
        std::cout << " " << argv[i];
    }
    std::cout << "\n";

    auto now = boost::posix_time::second_clock::local_time();
    std::cout << "# " << now << "\n";

    std::cout << sssp::arguments_csv_header << "," << sssp::dijkstra_result_csv_header << "\n";
    sssp::run_result result = sssp::execute_run(args);
    sssp::dijkstra_result_csv_values csv(result.result);
    std::cout << sssp::arguments_csv_values(args) << "," << csv << "\n";

#ifndef DISABLE_CAIRO
    if (args.image.size() > 0) {
        sssp::node_map<sssp::node_style> node_styles = result.graph.make_node_map([&](size_t i) {
            sssp::node_style style;
            style.position = result.positions[i];
            if (result.result[i].relaxation_phase == -1) {
                style.color = sssp::rgb(1.0, 0.5, 0.5);
            } else {
                double c =
                    0.25 + 0.75 * static_cast<double>(result.result[i].relaxation_phase) / (csv.relaxation_phases - 1);
                style.color = sssp::rgb(c, c, 1.0);
                style.text = std::to_string(result.result[i].relaxation_phase);
            }
            return style;
        });

        sssp::edge_map<sssp::edge_style> edge_styles =
            result.graph.make_edge_map([&](size_t source, size_t destination) {
                sssp::edge_style style;
                if (result.result[destination].predecessor == source) {
                    style.line_width *= 2;
                    style.color = sssp::rgb(0.25, 0.25, 1.0);
                    style.foreground = true;
                }
                return style;
            });

        try {
            int width = 800;
            int height = 800;
            auto surface = Cairo::PdfSurface::create(args.image, width, height);
            auto cr = Cairo::Context::create(surface);
            cr->translate(25, 25);
            cr->scale(width - 50, height - 50);
            cr->save();
            cr->set_source_rgb(1.0, 1.0, 1.0);
            cr->paint();
            cr->restore();
            draw_graph(cr, result.graph, node_styles, edge_styles);
        } catch (const std::ios_base::failure& ex) {
            std::cerr << "Could not write file " << args.image << ": " << ex.what() << "\n";
        }
    }
#endif

    return EXIT_SUCCESS;
}

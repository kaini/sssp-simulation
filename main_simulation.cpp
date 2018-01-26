#include "run.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

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

    sssp::execute_run(args, &std::cout, &std::cerr);

    return EXIT_SUCCESS;
}

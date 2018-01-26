#include "run.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cerr << "This tool does not take arguments.\n";
        std::cerr << "Pass each job as one line to the standard input as if you would call the simulation tool.\n";
        std::cerr << "WARNING: You most likely do *not* want to use this tool, use sssp-batch instead!\n";
        std::cerr << "         This tool is only single threaded!\n";
        return EXIT_FAILURE;
    }

    std::string job;
    do {
        std::getline(std::cin, job);
        if (std::cin.good()) {
            auto raw_args = boost::program_options::split_unix(job);
            auto opt_args = sssp::parse_arguments(argv[0], raw_args, nullptr);
            if (opt_args) {
                auto args = *opt_args;
                sssp::execute_run(args, &std::cout, &std::cerr);
            } else {
                std::cerr << "The job `" << job << "` is errornous, ignored!\n";
            }
        }
    } while (std::cin.good());

    return EXIT_SUCCESS;
}

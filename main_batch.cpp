#include "arguments.hpp"
#include "run.hpp"
#include <boost/assert.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/process.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace bi = boost::interprocess;
namespace bp = boost::process;
using boost::system::error_code;

namespace {

static constexpr size_t job_size = 1024;

enum class message_type {
    quit,
    job,
};

struct message {
    message_type type;
    union {
        char job[job_size];
    } payload;
};

struct remove_queue {
    ~remove_queue() {
        try {
            if (bi::message_queue::remove("sssp-batch-mq")) {
                std::cerr << "Deleted shared memory.\n";
            }
        } catch (...) {
        }
    }
};

} // namespace

static int worker_main() {
    bi::message_queue queue(bi::open_only, "sssp-batch-mq");
    remove_queue remove_queue;

    message msg;
    unsigned int msg_priority = 0;
    bi::message_queue::size_type msg_size = 0;
    while (true) {
        queue.receive(&msg, sizeof(msg), msg_size, msg_priority);
        BOOST_ASSERT(msg_size == sizeof(msg));
        if (msg.type == message_type::quit) {
            break;
        } else if (msg.type == message_type::job) {
            auto raw_args = boost::program_options::split_unix(msg.payload.job);
            auto opt_args = sssp::parse_arguments("", raw_args, nullptr);
            if (opt_args) {
                auto args = *opt_args;
                sssp::execute_run(args, &std::cout, &std::cerr);
            } else {
                std::cerr << "The job `" << msg.payload.job << "` is errornous, ignored!\n";
            }
        } else {
            BOOST_ASSERT(false);
        }
    }

    return EXIT_SUCCESS;
}

static int master_main(const std::string& basename) {
    int worker_count = std::thread::hardware_concurrency();

    error_code ec;
    auto self_exe = boost::dll::program_location(ec);
    if (ec) {
        std::cerr << "Could not find the own executable.\n";
        return EXIT_FAILURE;
    }

    bi::message_queue queue(bi::create_only, "sssp-batch-mq", worker_count * 100, sizeof(message));
    remove_queue remove_queue;

    std::cerr << "Starting " << worker_count << " workers ...\n";
    std::vector<bp::child> workers;
    for (int i = 0; i < worker_count; ++i) {
        try {
            std::ostringstream output_file;
            output_file << basename << "." << std::setw(4) << std::setfill('0') << (i + 1);
            workers.emplace_back(self_exe,
                                 (bp::args = {"worker"}),
                                 (bp::std_in < bp::close),
                                 (bp::std_out > output_file.str()),
                                 (bp::std_err > stderr));
        } catch (const bp::process_error& ex) {
            std::cerr << "Could not start a worker: " << ex.what() << "\n";
            return EXIT_FAILURE;
        }
    }

    std::ostringstream header_file;
    header_file << basename << "." << std::setw(4) << std::setfill('0') << 0;
    std::ofstream header_file_fp(header_file.str());
    header_file_fp << sssp::arguments_csv_header << "," << sssp::dijkstra_result_csv_header << "\n";
    header_file_fp.close();

    std::cerr << "Ready for input ...\n";
    std::string job;
    message msg;
    do {
        std::getline(std::cin, job);
        if (std::cin.good()) {
            msg.type = message_type::job;
            strncpy(msg.payload.job, job.c_str(), sizeof(msg.payload.job));
            queue.send(&msg, sizeof(msg), 0);
        }
    } while (std::cin.good());
    if (!std::cin.eof()) {
        std::cerr << "Standard input read error.\n";
    } else {
        std::cerr << "All jobs dispatched.\n";
    }

    std::cerr << "Waiting for workers ...\n";
    msg.type = message_type::quit;
    for (int i = 0; i < worker_count; ++i) {
        queue.send(&msg, sizeof(msg), 0);
    }
    for (auto& worker : workers) {
        try {
            worker.wait();
        } catch (const bp::process_error&) {
            // if I can't wait for a process it is already dead for some reason
        }
    }

    std::cerr << "Hint: Use `cat " << basename << ".*` to concatenate all output files.\n";
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "worker") == 0) {
        return worker_main();
    } else if (argc == 2 && strcmp(argv[1], "cleanup") == 0) {
        remove_queue remove_queue;
        return EXIT_SUCCESS;
    } else {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " OUTPUT_FILENAME\n\n";
            std::cerr << "Pass each job as one line to the standard input as if you would call the simulation tool.\n";
            std::cerr << "For each worker a file called OUPUT_FILENAME.0000 with 0000 being the worker number will be "
                         "created. An additional file with the CSV header will be created as well.\n";
            return EXIT_FAILURE;
        }
        try {
            std::cerr
                << "WARNING: When you cancel the program (e.g. by pressing CTRL+C) or it crashes you have to call `"
                << argv[0] << " cleanup` to free the shared memory!\n";
            std::cerr << "Hint: Use CTRL+D to send EOF to the standard input, this gracefully closes the program.\n";
            return master_main(argv[1]);
        } catch (const bi::interprocess_exception& ex) {
            std::cerr << "Could not communicate with other processes: " << ex.what() << "\n";
            std::cerr << "This most likely means that the shared memory was not cleaned up or you have wrong "
                         "permissions. Try calling `"
                      << argv[0] << " cleanup`.\n";
            return EXIT_FAILURE;
        }
    }
}

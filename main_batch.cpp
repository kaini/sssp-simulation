#include "arguments.hpp"
#include "dijkstra.hpp"
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/process.hpp>
#include <cstdlib>
#include <functional>
#include <memory>
#include <random>
#include <stdexcept>
#include <thread>

namespace bp = boost::process;
namespace asio = boost::asio;
using boost::system::error_code;

namespace {

class worker_error : public std::runtime_error {
  public:
    worker_error() : std::runtime_error("A worker aborted.") {}
};

struct worker {
    worker(const boost::filesystem::path& binary, asio::io_service& ios)
        : m_stdin(ios), m_stdout(ios),
          m_child(binary, (bp::std_in < m_stdin), (bp::std_out > m_stdout), (bp::std_err > stderr)) {
        recv_loop();
    }

    void post_job(std::shared_ptr<std::string> job) {
        BOOST_ASSERT(!m_done);
        m_jobs.push_back(job);
        if (!m_running) {
            send_loop();
        }
    }

    void post_done() {
        BOOST_ASSERT(!m_done);
        m_done = true;
        if (!m_running) {
            send_loop();
        }
    }

    void join() { m_child.join(); }

  private:
    void send_loop() {
        if (m_jobs.empty()) {
            m_running = false;
            if (m_done) {
                m_stdin.async_close();
            }
        } else {
            m_running = true;
            auto job = m_jobs.back();
            m_jobs.pop_back();
            // Note: job has to be kept alive via the lambda!
            asio::async_write(m_stdin, asio::buffer(*job), [this, job](error_code ec, size_t size) {
                if (ec) {
                    std::cerr << "ERROR: Could not dispatch job `" << *job << "`: " << ec << "\n";
                    throw worker_error();
                }
                // Fetch the next job
                send_loop();
            });
        }
    }

    void recv_loop() {
        asio::async_read_until(m_stdout, m_read_buffer, '\n', [this](error_code ec, size_t size) {
            if (ec == asio::error::eof) {
                // do nothing
            } else if (ec) {
                std::cerr << "ERROR: Could not read from worker: " << ec << "\n";
                throw worker_error();
            } else {
                std::istream is(&m_read_buffer);
                std::string line;
                std::getline(is, line);
                std::cout << line << "\n";
                recv_loop();
            }
        });
    }

    std::vector<std::shared_ptr<std::string>> m_jobs;
    bool m_done = false;
    bool m_running = false;
    asio::streambuf m_read_buffer;
    bp::async_pipe m_stdin;
    bp::async_pipe m_stdout;
    bp::child m_child;
};

} // namespace

// Note: This is a thread and *must not* access anything carelessly.
// Therefore this does only minimal (read only) work and dispatches
// everything to ios.post, which runs on the main thread -> no issues there.
static void stdin_thread(const std::vector<std::unique_ptr<worker>>& workers, asio::io_service& ios) {
    size_t at = 0;
    do {
        auto job = std::make_shared<std::string>();
        std::getline(std::cin, *job);
        *job += "\n";
        if (std::cin.good()) {
            ios.post(std::bind(&worker::post_job, workers[at].get(), job));
            at = (at + 1) % workers.size();
        }
    } while (std::cin.good());

    for (const auto& w : workers) {
        ios.post(std::bind(&worker::post_done, w.get()));
    }
}

int main(int argc, char* argv[]) {
    asio::io_service ios;

    std::vector<boost::filesystem::path> paths;
    error_code ec;
    auto pl = boost::dll::program_location(ec);
    if (!ec) {
        paths.push_back(pl.remove_filename());
    }
    paths.push_back(".");
    boost::filesystem::path binary = bp::search_path("sssp-worker", paths);
    if (binary.empty()) {
        std::cerr << "ERROR: Could not find the sssp-worker executable.\n";
        return EXIT_FAILURE;
    }

    unsigned int worker_count = std::thread::hardware_concurrency();
    std::cerr << "Starting " << worker_count << " workers ...\n";
    std::vector<std::unique_ptr<worker>> workers(worker_count);
    for (unsigned int i = 0; i < worker_count; ++i) {
        try {
            workers[i] = std::make_unique<worker>(binary, ios);
        } catch (const bp::process_error& ex) {
            std::cerr << "Could not start sssp-worker: " << ex.what() << "\n";
            return EXIT_FAILURE;
        }
    }

    std::cerr << "Ready for input ...\n";
    auto now = boost::posix_time::second_clock::local_time();
    std::cout << "# " << now << "\n";
    std::cout << sssp::arguments_csv_header << "," << sssp::dijkstra_result_csv_header << "\n";

    std::thread stdin_thread(&::stdin_thread, std::ref(workers), std::ref(ios));
    try {
        ios.run();
    } catch (const worker_error&) {
        std::cerr << "Aborting ...\n";
        return EXIT_FAILURE;
    }

    std::cerr << "Quitting ...\n";
    for (const auto& w : workers) {
        w->join();
    }
    stdin_thread.join();
    return EXIT_SUCCESS;
}

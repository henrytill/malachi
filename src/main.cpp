#include "project.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <span>
#include <string_view>
#include <system_error>
#include <variant>

#include <fcntl.h>
#include <getopt.h> // IWYU pragma: keep
#include <poll.h>   // IWYU pragma: keep
#include <signal.h> // NOLINT(hicpp-deprecated-headers,modernize-deprecated-headers)
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <git2/common.h>
#include <sqlite3.h>
#include <yyjson.h>

#include "config.h"
#include "db.h"
#include "filter.h"
#include "filter_mupdf.h"
#include "logging.h"
#include "parser.h"
#include "protocol.h"

using namespace malachi;

namespace
{

// Utilities

template <typename... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

constexpr auto kUsageMsg = std::string_view { "Usage: {} [-v|--version] [-c|--config] [-d|--debug]\n" };

struct Options
{
    bool version { false };
    bool config { false };
    bool debug { false };
};

void print_usage(char const *program)
{
    std::cerr << std::format(kUsageMsg, program);
}

// Signal handling

sig_atomic_t volatile loopstat = 1; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void handle_signal(int /*sig*/)
{
    loopstat = 0;
}

// Version printing

auto print_versions() -> int
{
    {
        int const major = MALACHI_VERSION_MAJOR;
        int const minor = MALACHI_VERSION_MINOR;
        int const patch = MALACHI_VERSION_PATCH;
        std::cout << std::format("malachi: {}.{}.{}\n", major, minor, patch);
    }
    {
        int major = 0;
        int minor = 0;
        int rev = 0;
        if (git_libgit2_version(&major, &minor, &rev) != 0)
        {
            std::cerr << std::format("Failed to get libgit2 version\n");
            return -1;
        }
        std::cout << std::format("libgit2: {}.{}.{}\n", major, minor, rev);
    }
    std::cout << std::format("sqlite: {}\n", sqlite3_libversion());
    std::cout << std::format("yyjson: {}\n", YYJSON_VERSION_STRING);
    for (auto const &filt : filter::global_registry().all())
    {
        std::cout << std::format("{}: {}\n", filt->name(), filt->version());
    }
    return 0;
}

// Command dispatch

auto handle_command(protocol::Command const &cmd) -> bool
{
    return std::visit(
        overloaded {
            [](protocol::AddCommand const &c) -> bool
            {
                logging::info("add: {}", c.path.string());
                return false;
            },
            [](protocol::RemoveCommand const &c) -> bool
            {
                logging::info("remove: {}", c.path.string());
                return false;
            },
            [](protocol::QueryCommand const &c) -> bool
            {
                logging::info(
                    "query: {} (id={}, filter={})",
                    c.terms,
                    c.query_id,
                    c.repo_filter ? c.repo_filter->string() : "");
                return false;
            },
            [](protocol::ShutdownCommand const &) -> bool
            {
                logging::info("shutdown requested");
                return true;
            },
        },
        cmd);
}

// Daemon loop (POSIX only)

constexpr mode_t kPipeMode = 0622; // NOLINT(misc-include-cleaner)

auto run_loop(std::filesystem::path const &pipe_path) -> int
{
    parser::Parser par;

    while (loopstat != 0)
    {
        // Open the named pipe non-blocking
        int pipe_fd = -1;
        while (loopstat != 0 && pipe_fd == -1)
        {
            pipe_fd = ::open(pipe_path.c_str(), O_RDONLY | O_NONBLOCK); // NOLINT(cppcoreguidelines-pro-type-vararg)
            if (pipe_fd == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                logging::error("open pipe: {}", std::strerror(errno));
                return -1;
            }
        }

        if (pipe_fd == -1)
        {
            break;
        }

        par.reset();

        struct pollfd pfd { .fd = pipe_fd, .events = POLLIN, .revents = 0 }; // NOLINT(misc-include-cleaner)

        while (loopstat != 0)
        {
            auto const rc = ::poll(&pfd, 1, 1000); // NOLINT(misc-include-cleaner)

            if (rc == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                logging::error("poll: {}", std::strerror(errno));
                ::close(pipe_fd);
                return -1;
            }

            if ((pfd.revents & POLLERR) != 0) // NOLINT(misc-include-cleaner)
            {
                logging::error("pipe error");
                ::close(pipe_fd);
                return -1;
            }

            if ((pfd.revents & POLLIN) != 0)
            {
                auto const n = par.feed(pipe_fd);
                if (n == 0)
                {
                    // EOF — client disconnected, reopen
                    break;
                }
                if (n < 0)
                {
                    if (errno == EINTR || errno == EAGAIN)
                    {
                        continue;
                    }
                    logging::error("read: {}", std::strerror(errno));
                    ::close(pipe_fd);
                    return -1;
                }

                // Drain all complete commands from the buffer
                while (true)
                {
                    auto result = par.next();
                    if (not result.has_value())
                    {
                        break;
                    }
                    if (std::holds_alternative<parser::ParseError>(*result))
                    {
                        logging::error("parse error: {}", std::get<parser::ParseError>(*result).reason);
                        continue;
                    }
                    auto const &cmd = std::get<protocol::Command>(*result);
                    if (handle_command(cmd))
                    {
                        loopstat = 0;
                    }
                }
            }

            if ((pfd.revents & POLLHUP) != 0) // NOLINT(misc-include-cleaner)
            {
                // Client disconnected — reopen the pipe
                break;
            }
        }

        ::close(pipe_fd);
    }

    return 0;
}

template <platform::Platform p = platform::get_platform()>
auto run(config::Config const &config) -> int
{
    if constexpr (p == platform::Platform::Windows)
    {
        std::cerr << "Daemon not supported on Windows\n";
        return EXIT_FAILURE;
    }

    // Register filters
    if (auto f = filter::make_mupdf_filter(); f != nullptr)
    {
        filter::global_registry().add(std::move(f));
    }

    // Set up signal handlers
    struct sigaction sa {};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    auto const sigint_rc = sigaction(SIGINT, &sa, nullptr);
    if (sigint_rc != 0)
    {
        logging::error("sigaction(SIGINT): {}", std::strerror(errno));
        return EXIT_FAILURE;
    }
    auto const sigterm_rc = sigaction(SIGTERM, &sa, nullptr);
    if (sigterm_rc != 0)
    {
        logging::error("sigaction(SIGTERM): {}", std::strerror(errno));
        return EXIT_FAILURE;
    }

    // Create runtime directory and named pipe
    auto const daemon_dir = config.runtime_dir / "malachi";
    std::error_code ec;
    std::filesystem::create_directories(daemon_dir, ec);
    if (ec)
    {
        logging::error("create runtime dir: {}", ec.message());
        return EXIT_FAILURE;
    }

    auto const pipe_path = daemon_dir / "command";
    auto const mkfifo_rc = ::mkfifo(pipe_path.c_str(), kPipeMode);
    if (mkfifo_rc == -1 && errno != EEXIST)
    {
        logging::error("mkfifo: {}", std::strerror(errno));
        return EXIT_FAILURE;
    }

    // Open database
    auto db_result = db::Database::open(config);
    if (std::holds_alternative<db::DbError>(db_result))
    {
        logging::error("open database: {}", std::get<db::DbError>(db_result).message);
        return EXIT_FAILURE;
    }

    logging::info("listening on {}", pipe_path.string());

    auto const rc = run_loop(pipe_path);

    // Clean up pipe
    std::filesystem::remove(pipe_path, ec);

    return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace

auto main(int argc, char *argv[]) -> int
try
{
    auto const args = std::span<char *> { argv, static_cast<size_t>(argc) };
    assert(not args.empty());

    auto opts = Options {};

    {
        constexpr auto long_options_len = size_t { 4 };
        // NOLINTBEGIN(misc-include-cleaner)
        constexpr auto long_options = std::array<struct option, long_options_len> {
            (struct option) { .name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'v' },
            (struct option) { .name = "config", .has_arg = no_argument, .flag = nullptr, .val = 'c' },
            (struct option) { .name = "debug", .has_arg = no_argument, .flag = nullptr, .val = 'd' },
            (struct option) { .name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0 },
        };
        // NOLINTEND(misc-include-cleaner)

        auto option_index = 0;

        while (true)
        {
            // NOLINTNEXTLINE(misc-include-cleaner)
            int const opt = getopt_long(
                static_cast<int>(args.size()),
                args.data(),
                "vcd",
                long_options.data(),
                &option_index);
            if (opt == -1)
            {
                break;
            }

            switch (opt)
            {
            case 'v':
                opts.version = true;
                break;
            case 'c':
                opts.config = true;
                break;
            case 'd':
                opts.debug = true;
                break;
            case '?':
                print_usage(args.front());
                return EXIT_FAILURE;
            default:
                break;
            }
        }
    }

    if (opts.debug)
    {
        logging::debug_enabled = true;
    }

    if (opts.version)
    {
        return print_versions() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    auto const config_result = config::Builder { std::getenv }.with_defaults().build();
    if (std::holds_alternative<config::Error>(config_result))
    {
        auto const &error = std::get<config::Error>(config_result);
        std::cerr << std::format("Failed to build config: {}\n", error.message);
        return EXIT_FAILURE;
    }

    auto const &config = std::get<config::Config>(config_result);

    if (opts.config)
    {
        std::cout << config.to_string();
        return EXIT_SUCCESS;
    }

    return run(config);
}
catch (std::exception const &e)
{
    std::cerr << std::format("Fatal error: {}\n", e.what());
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "Fatal error: Unknown exception\n";
    return EXIT_FAILURE;
}

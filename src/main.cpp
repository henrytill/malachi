#include <cstddef>
#include <format>
#include <iostream>
#include <span>
#include <string_view>

#include <getopt.h>
#include <unistd.h>

#include <git2/common.h>
#include <mupdf/fitz.h>
#include <sqlite3.h>

#include "config.h"
#include "platform.h"

using namespace malachi;

namespace {

constexpr size_t kCwdBufLen = 1024;
constexpr std::string_view kUsageMsg = "Usage: {} [--version] [--config] <query>\n";

struct Options {
  bool version{false};
  bool config{false};
};

void print_usage(const char *program) {
  std::cerr << std::format(kUsageMsg, program);
}

auto print_versions() -> int {
  {
    int major = 0;
    int minor = 0;
    int rev = 0;
    if (git_libgit2_version(&major, &minor, &rev) != 0) {
      std::cerr << std::format("Failed to get libgit2 version\n");
      return -1;
    }
    std::cout << std::format("libgit2: {}.{}.{}\n", major, minor, rev);
  }
  std::cout << std::format("mupdf: {}\n", FZ_VERSION);
  std::cout << std::format("sqlite: {}\n", sqlite3_libversion());
  return 0;
}

void print_config(const config::Config &config) {
  auto platform_str = platform::to_string_view(platform::get_platform());
  std::cout << std::format("platform: {}\n", platform_str);
  std::cout << std::format("config_dir: {}\n", config.config_dir.string());
  std::cout << std::format("data_dir: {}\n", config.data_dir.string());
}

} // namespace

auto main(int argc, char *argv[]) -> int try {
  const std::span<char *> args(argv, static_cast<size_t>(argc));
  if (args.size() == 1) {
    print_usage(args[0]);
    return EXIT_FAILURE;
  }

  Options opts{};
  {
    int option_index = 0;
    std::array<struct option, 3> long_options{
      {
        {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'v'},
        {.name = "config", .has_arg = no_argument, .flag = nullptr, .val = 'c'},
        {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0},
      },
    };

    while (true) {
      const int opt = getopt_long(static_cast<int>(args.size()), args.data(), "vc", long_options.data(), &option_index);
      if (opt == -1) {
        break;
      }

      switch (opt) {
      case 'v':
        opts.version = true;
        break;
      case 'c':
        opts.config = true;
        break;
      case '?':
        print_usage(args[0]);
        return EXIT_FAILURE;
      default:
        break;
      }
    }
  }

  if (opts.version) {
    return print_versions() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  auto config_result = config::Builder{getenv}.with_defaults().build();
  if (std::holds_alternative<config::Error>(config_result)) {
    const auto &error = std::get<config::Error>(config_result);
    std::cerr << std::format("Failed to build config: {}\n", error.message);
    return EXIT_FAILURE;
  }

  auto &config = std::get<config::Config>(config_result);

  if (opts.config) {
    print_config(config);
    return EXIT_SUCCESS;
  }

  auto offset = static_cast<size_t>(optind);
  if (offset < args.size()) {
    std::cout << "non-option argv elements:";
    for (const auto *arg : args.subspan(offset)) {
      std::cout << std::format(" {}", arg);
    }
    std::cout << '\n';
  }

  {
    std::array<char, kCwdBufLen> buf = {};
    if (auto *cwd = getcwd(buf.data(), buf.size())) {
      std::cout << std::format("cwd: {}\n", cwd);
    }
  }

  return EXIT_SUCCESS;
} catch (const std::exception &e) {
  std::cerr << std::format("Fatal error: {}\n", e.what());
  return EXIT_FAILURE;
} catch (...) {
  std::cerr << "Fatal error: Unknown exception\n";
  return EXIT_FAILURE;
}

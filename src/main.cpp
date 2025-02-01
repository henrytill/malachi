#include <cassert>
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

using namespace malachi;

namespace {

constexpr auto kUsageMsg = std::string_view{"Usage: {} [-v|--version] [-c|--config] <query>\n"};

struct Options {
  bool version{false};
  bool config{false};
};

void print_usage(const char *program) {
  std::cerr << std::format(kUsageMsg, program);
}

auto print_library_versions() -> int {
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

} // namespace

auto main(int argc, char *argv[]) -> int try {
  const auto args = std::span<char *>{argv, static_cast<size_t>(argc)};

  assert(not args.empty()); // we use args.front() below

  if (args.size() == 1) {
    print_usage(args.front());
    return EXIT_FAILURE;
  }

  Options opts{};

  {
    constexpr auto long_options_len = size_t{3};
    constexpr auto long_options = std::array<struct option, long_options_len>{
        {{.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'v'},
         {.name = "config", .has_arg = no_argument, .flag = nullptr, .val = 'c'},
         {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0}}};

    auto option_index = 0;

    while (true) {
      const int opt = getopt_long(static_cast<int>(args.size()), args.data(),
                                  "vc", long_options.data(), &option_index);
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
        print_usage(args.front());
        return EXIT_FAILURE;
      default:
        break;
      }
    }
  }

  if (opts.version) {
    return print_library_versions() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  const auto config_result = config::Builder{getenv}.with_defaults().build();
  if (std::holds_alternative<config::Error>(config_result)) {
    const auto &error = std::get<config::Error>(config_result);
    std::cerr << std::format("Failed to build config: {}\n", error.message);
    return EXIT_FAILURE;
  }

  const auto &config = std::get<config::Config>(config_result);

  if (opts.config) {
    std::cout << config.to_string();
    return EXIT_SUCCESS;
  }

  {
    auto offset = static_cast<size_t>(optind);
    if (offset < args.size()) {
      std::cout << "non-option argv elements:";
      for (const auto *arg : args.subspan(offset)) {
        std::cout << std::format(" {}", arg);
      }
      std::cout << '\n';
    }
  }

  {
    constexpr auto buf_len = size_t{1024};
    auto buf = std::array<char, buf_len>{};
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

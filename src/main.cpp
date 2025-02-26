#include "project.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <span>
#include <string_view>
#include <variant>

#include <getopt.h> // IWYU pragma: keep
#include <unistd.h>

#include <git2/common.h>
#include <sqlite3.h>

#ifdef MALACHI_HAVE_MUPDF
#  include <mupdf/fitz.h> // IWYU pragma: keep
#endif

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

#ifdef MALACHI_HAVE_MUPDF
inline void print_mupdf_version() {
  std::cout << std::format("mupdf: {}\n", FZ_VERSION); // NOLINT(misc-include-cleaner)
}
#else
inline void print_mupdf_version() {}
#endif

auto print_versions() -> int {
  {
    const int major = MALACHI_VERSION_MAJOR;
    const int minor = MALACHI_VERSION_MINOR;
    const int patch = MALACHI_VERSION_PATCH;
    std::cout << std::format("malachi: {}.{}.{}\n", major, minor, patch);
  }
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
  print_mupdf_version();
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

  auto opts = Options{};

  {
    constexpr auto long_options_len = size_t{3};
    // NOLINTBEGIN(misc-include-cleaner)
    constexpr auto long_options = std::array<struct option, long_options_len>{
        {{.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'v'},
         {.name = "config", .has_arg = no_argument, .flag = nullptr, .val = 'c'},
         {.name = nullptr, .has_arg = 0, .flag = nullptr, .val = 0}}};
    // NOLINTEND(misc-include-cleaner)

    auto option_index = 0;

    while (true) {
      // NOLINTNEXTLINE(misc-include-cleaner)
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
    return print_versions() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  const auto config_result = config::Builder{std::getenv}.with_defaults().build();
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
    auto offset = static_cast<size_t>(optind); // NOLINT(misc-include-cleaner)
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

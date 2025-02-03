#pragma once

#include <algorithm>

template <typename Derived>
struct Environment {
public:
  static constexpr auto getenv(const char *name) -> char * {
    const auto &vars = Derived::env;
    const auto *const iterator = std::ranges::find_if(vars, [name](const auto &pair) {
      return strcmp(pair.first, name) == 0;
    });
    if (iterator != vars.end()) {
      return const_cast<char *>(iterator->second); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }
    return nullptr;
  }

  friend Derived;

private:
  Environment() = default;
};

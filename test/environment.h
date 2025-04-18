#pragma once

#include <algorithm>
#include <string_view>

template <typename Derived>
struct Environment {
public:
  static constexpr auto getenv(const std::string_view name) -> char * {
    constexpr auto &vars = Derived::env;
    const auto *const iterator = std::ranges::find_if(vars, [name](const auto &pair) {
      return name == pair.first;
    });
    if (iterator != vars.end()) {
      return const_cast<char *>(iterator->second);
    }
    return nullptr;
  }

  friend Derived; // gives access to private constructor

private:
  Environment() = default; // private constructor to prevent direct instantiation
};

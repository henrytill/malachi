#pragma once

#include <algorithm>
#include <string_view>

template <typename Derived>
struct Environment {
public:
  static constexpr auto getenv(const std::string_view name) -> char * {
    const auto &vars = Derived::env;
    const auto *const iterator = std::ranges::find_if(vars, [name](const auto &pair) {
      return pair.first == name;
    });
    if (iterator != vars.end()) {
      return const_cast<char *>(iterator->second);
    }
    return nullptr;
  }

  friend Derived;

private:
  Environment() = default;
};

#pragma once

#include <algorithm>
#include <string_view>

template <typename Derived>
struct Environment {
public:
    static constexpr auto getenv(std::string_view const name) -> char * {
        constexpr auto &vars = Derived::env;
        auto const *const iterator = std::ranges::find_if(vars, [name](auto const &pair) {
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

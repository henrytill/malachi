#pragma once

#include <filesystem>
#include <string_view>

namespace malachi::status {

auto write(
    std::filesystem::path const &runtime_dir,
    std::filesystem::path const &repo_path,
    std::string_view sha) -> bool;

auto ensure(
    std::filesystem::path const &runtime_dir,
    std::filesystem::path const &repo_path) -> bool;

} // namespace malachi::status

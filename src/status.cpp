#include "status.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>

namespace malachi::status
{

namespace
{

auto repo_path_to_filename(std::filesystem::path const &repo_path) -> std::string
{
    auto filename = repo_path.string();
    std::ranges::replace(filename, '/', '_');
    return filename;
}

} // namespace

auto write(
    std::filesystem::path const &runtime_dir, // NOLINT(bugprone-easily-swappable-parameters)
    std::filesystem::path const &repo_path,
    std::string_view sha) -> bool
{
    auto const roots_dir = runtime_dir / "roots";
    std::error_code ec;
    std::filesystem::create_directories(roots_dir, ec);
    if (ec)
    {
        return false;
    }

    auto const status_file = roots_dir / repo_path_to_filename(repo_path);
    std::ofstream out { status_file };
    if (not out)
    {
        return false;
    }
    out << sha;
    return out.good();
}

auto ensure(
    std::filesystem::path const &runtime_dir, // NOLINT(bugprone-easily-swappable-parameters)
    std::filesystem::path const &repo_path) -> bool
{
    auto const roots_dir = runtime_dir / "roots";
    std::error_code ec;
    std::filesystem::create_directories(roots_dir, ec);
    if (ec)
    {
        return false;
    }

    auto const status_file = roots_dir / repo_path_to_filename(repo_path);
    if (not std::filesystem::exists(status_file))
    {
        std::ofstream { status_file };
    }
    return true;
}

} // namespace malachi::status

#include "status.h"

#include <filesystem>
#include <fstream>
#include <string_view>
#include <system_error>

namespace malachi::status
{

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

    // Use repo path as filename (replacing '/' with '_')
    auto filename = repo_path.string();
    for (auto &ch : filename)
    {
        if (ch == '/')
        {
            ch = '_';
        }
    }

    auto const status_file = roots_dir / filename;
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

    auto filename = repo_path.string();
    for (auto &ch : filename)
    {
        if (ch == '/')
        {
            ch = '_';
        }
    }

    auto const status_file = roots_dir / filename;
    if (not std::filesystem::exists(status_file))
    {
        std::ofstream { status_file };
    }
    return true;
}

} // namespace malachi::status

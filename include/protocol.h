#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <variant>

namespace malachi::protocol {

struct AddCommand {
    std::filesystem::path path;
};

struct RemoveCommand {
    std::filesystem::path path;
};

struct QueryCommand {
    std::string query_id;
    std::string terms;
    std::optional<std::filesystem::path> repo_filter;
};

struct ShutdownCommand {
};

using Command = std::variant<AddCommand, RemoveCommand, QueryCommand, ShutdownCommand>;

} // namespace malachi::protocol

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include <sqlite3.h>

#include "config.h"

namespace malachi::db {

struct DbError {
    int sqlite_code;
    std::string message;
};

template <typename T>
using Result = std::variant<T, DbError>;

struct Sqlite3Deleter {
    void operator()(sqlite3 *conn) const noexcept;
};

struct StmtDeleter {
    void operator()(sqlite3_stmt *stmt) const noexcept;
};

using Sqlite3Ptr = std::unique_ptr<sqlite3, Sqlite3Deleter>;
using StmtPtr = std::unique_ptr<sqlite3_stmt, StmtDeleter>;

class Database {
public:
    static auto open(config::Config const &config) -> Result<Database>;

    Database(Database const &) = delete;
    auto operator=(Database const &) -> Database & = delete;
    Database(Database &&) noexcept = default;
    auto operator=(Database &&) noexcept -> Database & = default;
    ~Database() = default;

    auto ensure_schema() -> std::optional<DbError>;
    auto repo_get(std::string_view repo_path) -> Result<std::optional<std::string>>;
    auto repo_set(std::string_view repo_path, std::string_view sha) -> std::optional<DbError>;

private:
    explicit Database(Sqlite3Ptr conn, std::filesystem::path path);

    Sqlite3Ptr conn_;
    std::filesystem::path path_;
};

} // namespace malachi::db

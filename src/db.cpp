#include "db.h"
#include "config.h"

#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <sqlite3.h>

#include "schema.h"

namespace malachi::db
{

void Sqlite3Deleter::operator()(sqlite3 *conn) const noexcept
{
    sqlite3_close(conn);
}

void StmtDeleter::operator()(sqlite3_stmt *stmt) const noexcept
{
    sqlite3_finalize(stmt);
}

Database::Database(Sqlite3Ptr conn, std::filesystem::path path)
    : conn_ { std::move(conn) }
    , path_ { std::move(path) }
{ }

auto Database::open(config::Config const &config) -> Result<Database>
{
    std::filesystem::create_directories(config.cache_dir);

    auto const db_path = config.cache_dir / "index.db";

    sqlite3 *raw = nullptr;
    auto const rc = sqlite3_open(db_path.c_str(), &raw);
    auto conn = Sqlite3Ptr { raw };

    if (rc != SQLITE_OK)
    {
        auto const *const msg = sqlite3_errmsg(conn.get());
        return DbError { .sqlite_code = rc, .message = std::format("sqlite3_open: {}", msg) };
    }

    auto db = Database { std::move(conn), db_path };

    if (auto err = db.ensure_schema(); err.has_value())
    {
        return *err;
    }

    return db;
}

auto Database::ensure_schema() -> std::optional<DbError>
{
    char *errmsg = nullptr;
    auto const rc = sqlite3_exec(conn_.get(), kMalachiSchemaSql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK)
    {
        auto const msg = std::string { errmsg != nullptr ? errmsg : "unknown error" };
        sqlite3_free(errmsg);
        return DbError { .sqlite_code = rc, .message = std::format("ensure_schema: {}", msg) };
    }
    return std::nullopt;
}

auto Database::repo_get(std::string_view repo_path) -> Result<std::optional<std::string>>
{
    static constexpr auto kSql = "SELECT root_hash FROM roots WHERE root_path = ?";

    sqlite3_stmt *raw = nullptr;
    auto const prc = sqlite3_prepare_v2(conn_.get(), kSql, -1, &raw, nullptr);
    auto stmt = StmtPtr { raw };

    if (prc != SQLITE_OK)
    {
        return DbError { .sqlite_code = prc, .message = std::format("repo_get prepare: {}", sqlite3_errmsg(conn_.get())) };
    }

    sqlite3_bind_text(stmt.get(), 1, repo_path.data(), static_cast<int>(repo_path.size()), SQLITE_STATIC);

    auto const src = sqlite3_step(stmt.get());
    if (src == SQLITE_DONE)
    {
        return std::optional<std::string> { std::nullopt };
    }
    if (src != SQLITE_ROW)
    {
        return DbError { .sqlite_code = src, .message = std::format("repo_get step: {}", sqlite3_errmsg(conn_.get())) };
    }

    auto const *text = reinterpret_cast<char const *>(sqlite3_column_text(stmt.get(), 0)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::optional<std::string> { text };
}

auto Database::repo_set(std::string_view repo_path, std::string_view sha) -> std::optional<DbError>
{
    static constexpr auto kSql = "INSERT OR REPLACE INTO roots (root_path, root_hash, updated_at)"
                                 " VALUES (?, ?, CURRENT_TIMESTAMP)";

    sqlite3_stmt *raw = nullptr;
    auto const prc = sqlite3_prepare_v2(conn_.get(), kSql, -1, &raw, nullptr);
    auto stmt = StmtPtr { raw };

    if (prc != SQLITE_OK)
    {
        return DbError { .sqlite_code = prc, .message = std::format("repo_set prepare: {}", sqlite3_errmsg(conn_.get())) };
    }

    sqlite3_bind_text(stmt.get(), 1, repo_path.data(), static_cast<int>(repo_path.size()), SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, sha.data(), static_cast<int>(sha.size()), SQLITE_STATIC);

    auto const src = sqlite3_step(stmt.get());
    if (src != SQLITE_DONE)
    {
        return DbError { .sqlite_code = src, .message = std::format("repo_set step: {}", sqlite3_errmsg(conn_.get())) };
    }

    return std::nullopt;
}

} // namespace malachi::db

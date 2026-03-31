#include <filesystem>
#include <optional>
#include <string>
#include <variant>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "config.h"
#include "db.h"

namespace fs = std::filesystem;

struct TempDirFixture
{
    fs::path tmp_dir;

    TempDirFixture()
    {
        tmp_dir = fs::temp_directory_path() / "malachi_db_test";
        fs::remove_all(tmp_dir);
        fs::create_directories(tmp_dir);
    }

    ~TempDirFixture()
    {
        fs::remove_all(tmp_dir);
    }

    [[nodiscard]] auto make_config() const -> malachi::config::Config
    {
        return malachi::config::Config {
            .config_dir = tmp_dir / "config",
            .data_dir = tmp_dir / "data",
            .cache_dir = tmp_dir / "cache",
            .runtime_dir = tmp_dir / "runtime",
        };
    }
};

TEST_CASE_METHOD(TempDirFixture, "Database::open creates database", "[db]")
{
    auto result = malachi::db::Database::open(make_config());
    REQUIRE(std::holds_alternative<malachi::db::Database>(result));
}

TEST_CASE_METHOD(TempDirFixture, "ensure_schema is idempotent", "[db]")
{
    auto result = malachi::db::Database::open(make_config());
    REQUIRE(std::holds_alternative<malachi::db::Database>(result));

    auto &db = std::get<malachi::db::Database>(result);
    auto err = db.ensure_schema();
    CHECK(not err.has_value());
}

TEST_CASE_METHOD(TempDirFixture, "repo_set and repo_get round-trip", "[db]")
{
    auto result = malachi::db::Database::open(make_config());
    REQUIRE(std::holds_alternative<malachi::db::Database>(result));

    auto &db = std::get<malachi::db::Database>(result);

    auto set_err = db.repo_set("/home/test/myrepo", "abc123");
    REQUIRE(not set_err.has_value());

    auto get_result = db.repo_get("/home/test/myrepo");
    REQUIRE(std::holds_alternative<std::optional<std::string>>(get_result));

    auto const &val = std::get<std::optional<std::string>>(get_result);
    REQUIRE(val.has_value());
    CHECK(*val == "abc123");
}

TEST_CASE_METHOD(TempDirFixture, "repo_get returns nullopt for missing path", "[db]")
{
    auto result = malachi::db::Database::open(make_config());
    REQUIRE(std::holds_alternative<malachi::db::Database>(result));

    auto &db = std::get<malachi::db::Database>(result);

    auto get_result = db.repo_get("/nonexistent/repo");
    REQUIRE(std::holds_alternative<std::optional<std::string>>(get_result));

    auto const &val = std::get<std::optional<std::string>>(get_result);
    CHECK(not val.has_value());
}

auto main(int argc, char *argv[]) -> int
{
    return Catch::Session().run(argc, argv);
}

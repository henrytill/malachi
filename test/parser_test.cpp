#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <variant>

#include <unistd.h>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "parser.h"
#include "protocol.h"

using namespace malachi;

// Write a length-prefixed JSON message to a file descriptor.
static void write_message(int fd, std::string const &json)
{
    auto const len = static_cast<uint32_t>(json.size());
    write(fd, &len, sizeof(len));
    write(fd, json.data(), json.size());
}

// RAII pipe pair
struct Pipe
{
    std::array<int, 2> fds {};

    Pipe()
    {
        pipe(fds.data()); // NOLINT(cppcoreguidelines-pro-type-vararg)
    }

    Pipe(Pipe const &) = delete;
    auto operator=(Pipe const &) -> Pipe & = delete;
    Pipe(Pipe &&) = delete;
    auto operator=(Pipe &&) -> Pipe & = delete;

    ~Pipe()
    {
        close(fds[0]);
        close(fds[1]);
    }

    [[nodiscard]] auto read_end() const -> int { return fds[0]; }
    [[nodiscard]] auto write_end() const -> int { return fds[1]; }
};

TEST_CASE("Parser: add command", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(), R"({"op":"add","path":"/home/test/repo"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    auto const &cmd = std::get<protocol::Command>(result_val);
    REQUIRE(std::holds_alternative<protocol::AddCommand>(cmd));
    CHECK(std::get<protocol::AddCommand>(cmd).path == "/home/test/repo");
}

TEST_CASE("Parser: remove command", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(), R"({"op":"remove","path":"/home/test/repo"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    auto const &cmd = std::get<protocol::Command>(result_val);
    REQUIRE(std::holds_alternative<protocol::RemoveCommand>(cmd));
    CHECK(std::get<protocol::RemoveCommand>(cmd).path == "/home/test/repo");
}

TEST_CASE("Parser: shutdown command", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(), R"({"op":"shutdown"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    auto const &cmd = std::get<protocol::Command>(result_val);
    CHECK(std::holds_alternative<protocol::ShutdownCommand>(cmd));
}

TEST_CASE("Parser: query without repoFilter", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(), R"({"op":"query","queryId":"q-001","terms":"hello world"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    auto const &cmd = std::get<protocol::Command>(result_val);
    REQUIRE(std::holds_alternative<protocol::QueryCommand>(cmd));
    auto const &qcmd = std::get<protocol::QueryCommand>(cmd);
    CHECK(qcmd.query_id == "q-001");
    CHECK(qcmd.terms == "hello world");
    CHECK(not qcmd.repo_filter.has_value());
}

TEST_CASE("Parser: query with repoFilter", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(),
                  R"({"op":"query","queryId":"q-002","terms":"foo","repoFilter":"/home/test/repo"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    auto const &cmd = std::get<protocol::Command>(result_val);
    REQUIRE(std::holds_alternative<protocol::QueryCommand>(cmd));
    auto const &qcmd = std::get<protocol::QueryCommand>(cmd);
    REQUIRE(qcmd.repo_filter.has_value());
    CHECK(qcmd.repo_filter.value() == "/home/test/repo"); // NOLINT(bugprone-unchecked-optional-access)
}

TEST_CASE("Parser: split feed yields nullopt then command", "[parser]")
{
    Pipe const p;
    auto const json = std::string { R"({"op":"shutdown"})" };
    auto const len = static_cast<uint32_t>(json.size());

    // Write only the length prefix first
    write(p.write_end(), &len, sizeof(len));

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    CHECK(not parser.next().has_value()); // incomplete — no JSON yet

    // Now write the JSON body
    write(p.write_end(), json.data(), json.size());
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();
    REQUIRE(result.has_value());
    auto const &result_val = result.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(result_val));
    CHECK(std::holds_alternative<protocol::ShutdownCommand>(std::get<protocol::Command>(result_val)));
}

TEST_CASE("Parser: malformed JSON produces ParseError", "[parser]")
{
    Pipe const p;
    // Write a length-prefixed payload that is not valid JSON
    auto const bad = std::string { "not json at all" };
    auto const len = static_cast<uint32_t>(bad.size());
    write(p.write_end(), &len, sizeof(len));
    write(p.write_end(), bad.data(), bad.size());

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);
    auto result = parser.next();

    REQUIRE(result.has_value());
    CHECK(std::holds_alternative<parser::ParseError>(result.value())); // NOLINT(bugprone-unchecked-optional-access)
}

TEST_CASE("Parser: multiple commands in one feed", "[parser]")
{
    Pipe const p;
    write_message(p.write_end(), R"({"op":"shutdown"})");
    write_message(p.write_end(), R"({"op":"add","path":"/tmp/repo"})");

    parser::Parser parser;
    REQUIRE(parser.feed(p.read_end()) > 0);

    auto first = parser.next();
    REQUIRE(first.has_value());
    auto const &first_val = first.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(first_val));
    CHECK(std::holds_alternative<protocol::ShutdownCommand>(std::get<protocol::Command>(first_val)));

    auto second = parser.next();
    REQUIRE(second.has_value());
    auto const &second_val = second.value(); // NOLINT(bugprone-unchecked-optional-access)
    REQUIRE(std::holds_alternative<protocol::Command>(second_val));
    CHECK(std::holds_alternative<protocol::AddCommand>(std::get<protocol::Command>(second_val)));
}

auto main(int argc, char *argv[]) -> int
{
    return Catch::Session().run(argc, argv);
}

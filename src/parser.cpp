#include "parser.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <unistd.h>

#include <yyjson.h>

#include "protocol.h"

namespace malachi::parser
{

// Declarative field descriptors

template <typename Cmd>
struct FieldSpec
{
    std::string_view key;
    bool required;
    void (*setter)(Cmd &, std::string_view);
};

template <typename Cmd, std::size_t N>
static auto apply_fields(
    yyjson_val *obj,
    Cmd &cmd,
    FieldSpec<Cmd> const (&specs)[N]) -> std::optional<ParseError>
{
    for (auto const &spec : specs)
    {
        auto *val = yyjson_obj_getn(obj, spec.key.data(), spec.key.size());
        if (val == nullptr || yyjson_get_type(val) != YYJSON_TYPE_STR)
        {
            if (spec.required)
            {
                return ParseError { std::format("missing required field '{}'", spec.key) };
            }
        }
        else
        {
            spec.setter(cmd, std::string_view { yyjson_get_str(val), yyjson_get_len(val) });
        }
    }
    return std::nullopt;
}

// Per-command field tables

static constexpr FieldSpec<protocol::AddCommand> kAddFields[] = {
    { "path", true, [](protocol::AddCommand &c, std::string_view v)
      { c.path = v; } },
};

static constexpr FieldSpec<protocol::RemoveCommand> kRemoveFields[] = {
    { "path", true, [](protocol::RemoveCommand &c, std::string_view v)
      { c.path = v; } },
};

static constexpr FieldSpec<protocol::QueryCommand> kQueryFields[] = {
    { "queryId", true, [](protocol::QueryCommand &c, std::string_view v)
      { c.query_id = v; } },
    { "terms", true, [](protocol::QueryCommand &c, std::string_view v)
      { c.terms = v; } },
    { "repoFilter", false, [](protocol::QueryCommand &c, std::string_view v)
      { c.repo_filter = v; } },
};

// Parser implementation

Parser::Parser(std::size_t max_buffer)
    : buf_(max_buffer)
{
}

auto Parser::feed(int fd) -> ssize_t
{
    auto const space = buf_.size() - buf_used_;
    if (space == 0)
    {
        return -1; // buffer full
    }
    auto const n = ::read(fd, buf_.data() + buf_used_, space);
    if (n > 0)
    {
        buf_used_ += static_cast<std::size_t>(n);
    }
    return n;
}

auto Parser::next() -> ParseResult
{
    if (state_ == State::kLength)
    {
        if (buf_used_ < sizeof(uint32_t))
        {
            return std::nullopt;
        }
        std::memcpy(&json_len_, buf_.data(), sizeof(uint32_t));
        compact(sizeof(uint32_t));
        state_ = State::kJson;
    }

    // State::kJson
    if (buf_used_ < json_len_)
    {
        return std::nullopt;
    }

    auto result = parse_json(std::span<std::byte const> { buf_.data(), json_len_ });
    compact(json_len_);
    state_ = State::kLength;
    json_len_ = 0;
    return result;
}

void Parser::reset()
{
    buf_used_ = 0;
    state_ = State::kLength;
    json_len_ = 0;
}

auto Parser::parse_json(std::span<std::byte const> json_bytes)
    -> std::variant<protocol::Command, ParseError>
{
    auto *doc = yyjson_read(
        reinterpret_cast<char const *>(json_bytes.data()),
        json_bytes.size(),
        0);

    if (doc == nullptr)
    {
        return ParseError { "invalid JSON" };
    }

    struct DocGuard
    {
        yyjson_doc *doc;
        ~DocGuard() { yyjson_doc_free(doc); }
    } guard { doc };

    auto *root = yyjson_doc_get_root(doc);
    if (root == nullptr || yyjson_get_type(root) != YYJSON_TYPE_OBJ)
    {
        return ParseError { "JSON root is not an object" };
    }

    auto *op_val = yyjson_obj_get(root, "op");
    if (op_val == nullptr || yyjson_get_type(op_val) != YYJSON_TYPE_STR)
    {
        return ParseError { "missing 'op' field" };
    }

    auto const op = std::string_view { yyjson_get_str(op_val), yyjson_get_len(op_val) };

    if (op == "add")
    {
        protocol::AddCommand cmd;
        if (auto err = apply_fields(root, cmd, kAddFields); err.has_value())
        {
            return *err;
        }
        return cmd;
    }

    if (op == "remove")
    {
        protocol::RemoveCommand cmd;
        if (auto err = apply_fields(root, cmd, kRemoveFields); err.has_value())
        {
            return *err;
        }
        return cmd;
    }

    if (op == "query")
    {
        protocol::QueryCommand cmd;
        if (auto err = apply_fields(root, cmd, kQueryFields); err.has_value())
        {
            return *err;
        }
        return cmd;
    }

    if (op == "shutdown")
    {
        return protocol::ShutdownCommand {};
    }

    return ParseError { std::format("unknown op '{}'", op) };
}

void Parser::compact(std::size_t skip)
{
    if (skip >= buf_used_)
    {
        buf_used_ = 0;
        return;
    }
    auto const remaining = buf_used_ - skip;
    std::memmove(buf_.data(), buf_.data() + skip, remaining);
    buf_used_ = remaining;
}

} // namespace malachi::parser

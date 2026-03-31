#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#ifndef _WIN32
#    include <sys/types.h>
#endif

#include "protocol.h"

namespace malachi::parser
{

struct ParseError
{
    std::string reason;
};

// nullopt        -> need more data
// Command        -> successfully parsed one command
// ParseError     -> malformed record (buffer advanced past it)
using ParseResult = std::optional<std::variant<protocol::Command, ParseError>>;

class Parser
{
public:
    static constexpr std::size_t kDefaultBufferSize = 65536;

    explicit Parser(std::size_t max_buffer = kDefaultBufferSize);

    // Feed bytes from fd into the buffer. Returns bytes read, 0 on EOF, -1 on error.
    [[nodiscard]] auto feed(int fd) -> ssize_t;

    // Extract the next complete command from the buffer.
    [[nodiscard]] auto next() -> ParseResult;

    // Reset parser state (call after pipe EOF/reconnect).
    void reset();

private:
    enum class State : uint8_t
    {
        kLength,
        kJson,
    };

    std::vector<std::byte> buf_;
    std::size_t buf_used_ { 0 };
    State state_ { State::kLength };
    uint32_t json_len_ { 0 };

    [[nodiscard]] static auto parse_json(std::span<std::byte const> json_bytes) -> std::variant<protocol::Command, ParseError>;
    void compact(std::size_t skip);
};

} // namespace malachi::parser

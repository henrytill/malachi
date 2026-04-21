#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>

#include "platform.h"

namespace malachi::config {

enum class ErrorCode : int8_t {
    kMissingDir = 1,
};

struct Error {
    ErrorCode code;
    std::string message;
};

struct Config {
    std::filesystem::path config_dir;
    std::filesystem::path data_dir;
    std::filesystem::path cache_dir;
    std::filesystem::path runtime_dir;

    static auto platform() -> platform::Platform;
    [[nodiscard]] auto to_string() const -> std::string;
};

using Result = std::variant<Config, Error>;

class Builder {
public:
    explicit Builder(platform::GetEnvFn getenv);
    auto with_defaults() && -> Builder &&;
    auto build() && -> Result;

private:
    platform::GetEnvFn getenv_;
    std::optional<std::filesystem::path> maybe_config_dir_;
    std::optional<std::filesystem::path> maybe_data_dir_;
    std::optional<std::filesystem::path> maybe_cache_dir_;
    std::optional<std::filesystem::path> maybe_runtime_dir_;
};

} // namespace malachi::config

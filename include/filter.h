#pragma once

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace malachi::filter
{

class Filter
{
public:
    virtual ~Filter() = default;

    [[nodiscard]] virtual auto name() const -> std::string_view = 0;
    [[nodiscard]] virtual auto extensions() const -> std::span<std::string_view const> = 0;
    [[nodiscard]] virtual auto version() const -> std::string_view = 0;

    virtual auto extract(std::string const &input_path) -> std::optional<std::string> = 0;
};

class Registry
{
public:
    void add(std::unique_ptr<Filter> filter);

    [[nodiscard]] auto find_by_extension(std::string_view ext) const -> Filter const *;
    [[nodiscard]] auto all() const -> std::span<std::unique_ptr<Filter> const>;

private:
    std::vector<std::unique_ptr<Filter>> filters_;
};

auto global_registry() -> Registry &;

} // namespace malachi::filter

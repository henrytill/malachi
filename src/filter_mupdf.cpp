#include <array>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include <mupdf/fitz.h> // IWYU pragma: keep

#include "filter.h"

namespace malachi::filter {

namespace {

class MupdfFilter final : public Filter {
public:
    [[nodiscard]]
    auto name() const -> std::string_view override
    {
        return "mupdf";
    }

    [[nodiscard]]
    auto extensions() const -> std::span<std::string_view const> override
    {
        static constexpr std::array<std::string_view, 2> kExts { ".pdf", ".PDF" };
        return kExts;
    }

    [[nodiscard]]
    auto version() const -> std::string_view override
    {
        return FZ_VERSION; // NOLINT(misc-include-cleaner)
    }

    [[nodiscard]]
    auto extract(std::string const & /*input_path*/) const -> std::optional<std::string> override
    {
        return std::nullopt; // Not yet implemented
    }
};

struct Registrar {
    Registrar() { global_registry().add(std::make_unique<MupdfFilter>()); }
} const registrar;

} // namespace

} // namespace malachi::filter

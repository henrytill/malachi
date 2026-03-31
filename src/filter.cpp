#include "filter.h"

#include <algorithm>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace malachi::filter
{

void Registry::add(std::unique_ptr<Filter> filter)
{
    filters_.push_back(std::move(filter));
}

auto Registry::find_by_extension(std::string_view ext) const -> Filter const *
{
    for (auto const &filter : filters_)
    {
        auto const exts = filter->extensions();
        auto const it = std::ranges::find(exts, ext);
        if (it != exts.end())
        {
            return filter.get();
        }
    }
    return nullptr;
}

auto Registry::all() const -> std::span<std::unique_ptr<Filter> const>
{
    return filters_;
}

auto global_registry() -> Registry &
{
    static Registry instance;
    return instance;
}

} // namespace malachi::filter

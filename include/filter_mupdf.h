#pragma once

#include <memory>

#include "filter.h"

namespace malachi::filter
{

auto make_mupdf_filter() -> std::unique_ptr<Filter>;

} // namespace malachi::filter

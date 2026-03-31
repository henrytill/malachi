#pragma once

#ifdef MALACHI_HAVE_MUPDF

#    include <memory>

#    include "filter.h"

namespace malachi::filter
{

auto make_mupdf_filter() -> std::unique_ptr<Filter>;

} // namespace malachi::filter

#endif // MALACHI_HAVE_MUPDF

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "config.h"

using malachi::config::Config;
using malachi::config::ConfigBuilder;

namespace {

TEST_CASE("ConfigBuilder builds a Config", "[ConfigBuilder]")
{
    auto config = Config{};
    auto config_builder = ConfigBuilder{};
    auto result = config_builder.with_defaults(std::getenv).build(config);
    REQUIRE(result == ConfigBuilder::Result::Success);
}

} // namespace

int main(int argc, char *argv[])
{
    const int result = Catch::Session().run(argc, argv);
    return result;
}

#include "Config.h"

#include <cassert>
#include <memory>

#include "Platform.h"

using std::filesystem::path;

namespace malachi::config {

template <Platform p>
std::optional<path> get_config_dir(GetEnv getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        auto app_data = std::unique_ptr<char>{getenv("APPDATA")};
        if (app_data == nullptr) {
            return std::nullopt;
        }
        const auto config_dir = path{app_data.release()};
        return std::optional<path>{config_dir / name};
    } else if constexpr (p == Platform::MacOS) {
        auto home = std::unique_ptr<char>{getenv("HOME")};
        if (home == nullptr) {
            return std::nullopt;
        }
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / "Library" / "Application Support" / name};
    } else {
        // Linux and other Unixen
        auto xdg_config_home = std::unique_ptr<char>{getenv("XDG_CONFIG_HOME")};
        if (xdg_config_home == nullptr) {
            auto home = std::unique_ptr<char>{getenv("HOME")};
            if (home == nullptr) {
                return std::nullopt;
            }
            auto home_dir = path{home.release()};
            return std::optional<path>{home_dir / ".config" / name};
        }
        const auto config_dir = path{xdg_config_home.release()};
        return std::optional<path>{config_dir / name};
    }
}

template <Platform p>
std::optional<path> get_data_dir(GetEnv getenv, const path name)
{
    assert(name.empty() == false);

    if constexpr (p == Platform::Windows) {
        auto local_app_data = std::unique_ptr<char>{getenv("LOCALAPPDATA")};
        if (local_app_data == nullptr) {
            return std::nullopt;
        }
        const auto data_dir = path{local_app_data.release()};
        return std::optional<path>{data_dir / name};
    } else if constexpr (p == Platform::MacOS) {
        auto home = std::unique_ptr<char>{getenv("HOME")};
        if (home == nullptr) {
            return std::nullopt;
        }
        const auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / "Library" / "Application Support" / name};
    } else {
        // Linux and other Unixen
        auto xdg_data_home = std::unique_ptr<char>{getenv("XDG_DATA_HOME")};
        if (xdg_data_home == nullptr) {
            auto home = std::unique_ptr<char>{getenv("HOME")};
            if (home == nullptr) {
                return std::nullopt;
            }
            auto home_dir = path{home.release()};
            return std::optional<path>{home_dir / ".local" / "share" / name};
        }
        const auto data_dir = path{xdg_data_home.release()};
        return std::optional<path>{data_dir / name};
    }
}

ConfigBuilder &ConfigBuilder::with_defaults(GetEnv getenv)
{
    constexpr Platform platform = get_platform();
    const auto name = path{"malachi"};
    maybe_config_dir_ = get_config_dir<platform>(getenv, name);
    maybe_data_dir_ = get_data_dir<platform>(getenv, name);
    return *this;
}

ConfigBuilder::Result ConfigBuilder::build(Config &cfg)
{
    if (!maybe_config_dir_.has_value()) {
        return Result::MissingConfigDir;
    }
    if (!maybe_data_dir_.has_value()) {
        return Result::MissingDataDir;
    }
    cfg.config_dir = maybe_config_dir_.value();
    cfg.data_dir = maybe_data_dir_.value();
    return Result::Success;
}

std::string to_string(const ConfigBuilder::Result result)
{
    switch (result) {
    case ConfigBuilder::Result::Success:
        return "Success";
    case ConfigBuilder::Result::MissingConfigDir:
        return "Missing config directory";
    case ConfigBuilder::Result::MissingDataDir:
        return "Missing data directory";
    }
    assert(false);
}

} // namespace malachi::config

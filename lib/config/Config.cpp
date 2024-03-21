#include "Config.h"

#include <cassert>
#include <memory>

#include "Platform.h"

using std::filesystem::path;

namespace malachi::config {

std::optional<path> get_windows_config_dir(GetEnv getenv)
{
    auto app_data = std::unique_ptr<char>{getenv("APPDATA")};
    if (app_data == nullptr) {
        return std::nullopt;
    }
    const auto config_dir = path{app_data.release()};
    return std::optional<path>{config_dir / "malachi"};
}

std::optional<path> get_windows_data_dir(GetEnv getenv)
{
    auto local_app_data = std::unique_ptr<char>{getenv("LOCALAPPDATA")};
    if (local_app_data == nullptr) {
        return std::nullopt;
    }
    const auto data_dir = path{local_app_data.release()};
    return std::optional<path>{data_dir / "malachi"};
}

std::optional<path> get_macos_support_dir(GetEnv getenv)
{
    auto home = std::unique_ptr<char>{getenv("HOME")};
    if (home == nullptr) {
        return std::nullopt;
    }
    const auto home_dir = path{home.release()};
    return std::optional<path>{home_dir / "Library" / "Application Support" / "malachi"};
}

std::optional<path> get_xdg_config_home(GetEnv getenv)
{
    auto xdg_config_home = std::unique_ptr<char>{getenv("XDG_CONFIG_HOME")};
    if (xdg_config_home == nullptr) {
        auto home = std::unique_ptr<char>{getenv("HOME")};
        if (home == nullptr) {
            return std::nullopt;
        }
        auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / ".config" / "malachi"};
    }
    const auto config_dir = path{xdg_config_home.release()};
    return std::optional<path>{config_dir / "malachi"};
}

std::optional<path> get_xdg_data_home(GetEnv getenv)
{
    auto xdg_data_home = std::unique_ptr<char>{getenv("XDG_DATA_HOME")};
    if (xdg_data_home == nullptr) {
        auto home = std::unique_ptr<char>{getenv("HOME")};
        if (home == nullptr) {
            return std::nullopt;
        }
        auto home_dir = path{home.release()};
        return std::optional<path>{home_dir / ".local" / "share" / "malachi"};
    }
    const auto data_dir = path{xdg_data_home.release()};
    return std::optional<path>{data_dir / "malachi"};
}

ConfigBuilder &ConfigBuilder::with_defaults(Platform platform, GetEnv getenv)
{
    switch (platform) {
    case Platform::Windows: {
        maybe_config_dir_ = get_windows_config_dir(getenv);
        maybe_data_dir_ = get_windows_data_dir(getenv);
        break;
    }
    case Platform::MacOS: {
        const auto support_dir = get_macos_support_dir(getenv);
        maybe_config_dir_ = support_dir;
        maybe_data_dir_ = support_dir;
        break;
    }
    case Platform::Linux:
    case Platform::Unknown:
    default: {
        maybe_config_dir_ = get_xdg_config_home(getenv);
        maybe_data_dir_ = get_xdg_data_home(getenv);
        break;
    }
    }
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

const char *to_string(ConfigBuilder::Result result)
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

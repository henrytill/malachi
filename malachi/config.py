from dataclasses import dataclass
from pathlib import Path

import platformdirs


@dataclass
class Config:
    """Platform-specific directory paths for malachi."""

    configdir: Path
    datadir: Path
    cachedir: Path
    runtimedir: Path

    @classmethod
    def from_platformdirs(cls, appname: str = "malachi") -> "Config":
        """Create Config using platform-specific directories."""
        return cls(
            configdir=platformdirs.user_config_path(appname),
            datadir=platformdirs.user_data_path(appname),
            cachedir=platformdirs.user_cache_path(appname),
            runtimedir=platformdirs.user_runtime_path(appname),
        )

"""Main entry point."""

import sys
from typing import Sequence

import platformdirs

from . import version


def main(args: Sequence[str] = sys.argv[1:]) -> int:
    """The main entry point for the command-line interface.

    Returns:
        An exit code.
    """
    if args in (["--version"], ["-v"]):
        print(f"{version.__version__}")
        return 0

    appname = "malachi"
    appauthor = "henrytill"
    config_dir = platformdirs.user_config_path(appname, appauthor)
    data_dir = platformdirs.user_data_path(appname, appauthor)

    print(f"config_dir: {config_dir}")
    print(f"data_dir: {data_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

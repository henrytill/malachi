"""Main entry point."""

import sys
from typing import Sequence

import fitz  # type: ignore
import platformdirs
import pygit2

from . import version


def _main(args: Sequence[str]) -> int:
    if args in (["--version"], ["-v"]):
        print(f"{version.__version__}")
        return 0

    appname = "malachi"
    appauthor = "henrytill"
    config_dir = platformdirs.user_config_path(appname, appauthor)
    data_dir = platformdirs.user_data_path(appname, appauthor)

    print(f"config_dir: {config_dir}")
    print(f"data_dir: {data_dir}")
    print(f"libgit2: {pygit2.LIBGIT2_VERSION}")  # pylint: disable=no-member
    print(f"fitz: {fitz.version[0]}")  # pyright: ignore
    return 0


def main() -> int:
    """The main entry point for the command-line interface.

    Returns:
        An exit code.
    """
    return _main(sys.argv[1:])


if __name__ == "__main__":
    sys.exit(main())

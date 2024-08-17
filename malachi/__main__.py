"""Main entry point."""

import sys
from typing import Sequence

from . import version


def main(args: Sequence[str] = sys.argv[1:]) -> int:
    """The main entry point for the command-line interface.

    Returns:
        An exit code.
    """
    print("Hello, world!")
    print(f"{version.__version__}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

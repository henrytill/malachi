import argparse
import json
import logging
import os
import platform
import selectors
import shutil
import signal
import sqlite3
import subprocess
import sys
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Any, Callable, Optional

from malachi import __version__
from malachi.config import Config
from malachi.database import Database
from malachi.status import writestatus

MAXOPSIZE = 16
MAXHASHLEN = 65
MAXUUIDLEN = 40
MAXQUERYLEN = 4096
MAXJSONOVERHEAD = 200
MAXLINESIZE = (
    MAXOPSIZE
    + (2 * os.pathconf("/", "PC_PATH_MAX"))
    + (2 * MAXHASHLEN)
    + MAXUUIDLEN
    + MAXQUERYLEN
    + MAXJSONOVERHEAD
)

_git_path: Optional[str] = None


def git_path() -> Optional[str]:
    """Get cached git executable path."""
    global _git_path  # pylint: disable=global-statement
    if _git_path is None:
        _git_path = shutil.which("git")
        if _git_path is None:
            logging.error("git command not found")
    return _git_path


class Operation(Enum):
    """Command operations."""

    ADD = "add"
    REMOVE = "remove"
    QUERY = "query"
    SHUTDOWN = "shutdown"


@dataclass
class Command:
    """JSONL command structure."""

    op: Operation
    data: dict[str, Any]


class Parser:
    """JSONL protocol parser."""

    def __init__(self, bufsize: int):
        self.bufsize = bufsize
        self.buf = bytearray()

    def reset(self):
        """Reset parser state."""
        self.buf.clear()

    def input(self, fd: int) -> int:
        """Read data from file descriptor into buffer."""
        space = self.bufsize - len(self.buf) - 1
        if space == 0:
            raise BufferError("Parser buffer full")

        data = os.read(fd, space)
        if not data:
            return 0

        self.buf.extend(data)
        return len(data)

    # pylint: disable=too-many-return-statements
    def parse_command(self) -> Optional[Command]:
        """Parse next JSONL command from buffer."""
        if not self.buf:
            return None

        newline_pos = self.buf.find(b"\n")
        if newline_pos == -1:
            return None

        line = self.buf[:newline_pos].decode("utf-8")
        self.buf = self.buf[newline_pos + 1 :]

        try:
            data = json.loads(line)
            if not isinstance(data, dict):
                logging.error(
                    "Command must be JSON object, got %s", type(data).__name__
                )
                return None

            op_str = data.get("op")
            if not op_str:
                logging.error("Command missing 'op' field")
                return None

            try:
                op = Operation(op_str)
            except ValueError:
                logging.error("Unknown operation: %s", op_str)
                return None

            return Command(op=op, data=data)
        except json.JSONDecodeError as e:
            logging.error("Invalid JSON: %s", e)
            return None


def get_git_head(repo_path: Path) -> Optional[str]:
    """Get current HEAD commit hash."""
    git = git_path()
    if not git:
        return None

    try:
        result = subprocess.run(
            [git, "-C", repo_path, "rev-parse", "HEAD"],
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        logging.error("Failed to get HEAD for %s: %s", repo_path, e)
        return None


def index_repository_initial(db: Database, repo_path: Path, head_hash: str) -> bool:
    """Index all files in repository for first time."""
    git = git_path()
    if not git:
        return False

    try:
        db.set_repo_hash(repo_path, head_hash)

        root_id = db.get_repo_id(repo_path)
        if root_id is None:
            logging.error("Failed to get repo ID for %s", repo_path)
            return False

        result = subprocess.run(
            [
                git,
                "-C",
                repo_path,
                "ls-tree",
                "-r",
                "--format=%(path) %(objectname) %(objectsize)",
                head_hash,
            ],
            capture_output=True,
            text=True,
            check=True,
        )

        for line in result.stdout.splitlines():
            parts = line.split(" ", 2)
            if len(parts) != 3:
                continue
            path, objhash, size = parts
            db.add_leaf(root_id, path, objhash, int(size))

        return True

    except subprocess.CalledProcessError as e:
        logging.error("Failed to index repository %s: %s", repo_path, e)
        return False


# pylint: disable=too-many-locals,too-many-branches
def index_repository_incremental(
    db: Database, repo_path: Path, old_hash: str, new_hash: str
) -> bool:
    """Update index with changes between two commits."""
    git = git_path()
    if not git:
        return False

    try:
        result = subprocess.run(
            [git, "-C", repo_path, "diff-tree", "--raw", old_hash, new_hash],
            capture_output=True,
            text=True,
            check=True,
        )

        root_id = db.get_repo_id(repo_path)
        if root_id is None:
            logging.error("Failed to get repo ID for %s", repo_path)
            return False

        changes = []
        for line in result.stdout.splitlines():
            fields = line.split("\t", 1)
            if len(fields) != 2:
                continue

            info, path = fields
            parts = info.split()
            if len(parts) < 5:
                continue

            status = parts[4]
            objhash = parts[2] if status == "D" else parts[3]
            changes.append((status, path, objhash))

        hashes_needing_size = [
            objhash for status, _, objhash in changes if status in ("A", "M")
        ]

        sizes = {}
        if hashes_needing_size:
            batch_input = "\n".join(hashes_needing_size)
            size_result = subprocess.run(
                [git, "-C", repo_path, "cat-file", "--batch-check"],
                input=batch_input,
                capture_output=True,
                text=True,
                check=True,
            )
            for line in size_result.stdout.splitlines():
                parts = line.split()
                if len(parts) == 3:
                    sizes[parts[0]] = int(parts[2])

        for status, path, objhash in changes:
            if status == "A":
                size = sizes.get(objhash, 0)
                db.add_leaf(root_id, path, objhash, size)
            elif status == "M":
                db.update_leaf(root_id, path, objhash)
            elif status == "D":
                db.remove_leaf(root_id, path)

        db.set_repo_hash(repo_path, new_hash)
        return True

    except subprocess.CalledProcessError as e:
        logging.error("Failed to update repository %s: %s", repo_path, e)
        return False


# pylint: disable=too-many-return-statements,too-many-nested-blocks
def handle_command(config: Config, db: Database, cmd: Command) -> bool:
    """Handle parsed command. Returns True to shutdown."""
    match cmd.op:
        case Operation.ADD:
            path_str = cmd.data.get("path")
            if not path_str:
                logging.error("add missing 'path' field")
                return False

            repo_path = Path(path_str)
            logging.info("Add repository: %s", repo_path)

            head_hash = get_git_head(repo_path)
            if not head_hash:
                return False

            cached_hash = db.get_repo_hash(repo_path)

            if cached_hash is None:
                logging.info("Initial indexing of %s at %s", repo_path, head_hash)
                if not index_repository_initial(db, repo_path, head_hash):
                    logging.error("Failed to index repository %s", repo_path)
                    return False
                writestatus(config.runtimedir, str(repo_path), head_hash)
            elif cached_hash != head_hash:
                logging.info(
                    "Updating %s from %s to %s", repo_path, cached_hash, head_hash
                )
                if not index_repository_incremental(
                    db, repo_path, cached_hash, head_hash
                ):
                    logging.error("Failed to update repository %s", repo_path)
                    return False
                writestatus(config.runtimedir, str(repo_path), head_hash)
            else:
                logging.info(
                    "Repository %s already up to date at %s", repo_path, head_hash
                )

            return False
        case Operation.REMOVE:
            path_str = cmd.data.get("path")
            if not path_str:
                logging.error("remove missing 'path' field")
                return False
            repo_path = Path(path_str)
            logging.info("Remove repository: %s", repo_path)
            return False
        case Operation.QUERY:
            query_id = cmd.data.get("query_id")
            terms = cmd.data.get("terms")
            if not query_id or not terms:
                logging.error("query missing 'query_id' or 'terms' field")
                return False
            repo_filter = cmd.data.get("repo_filter")
            logging.info("Query: %s (id=%s, filter=%s)", terms, query_id, repo_filter)
            return False
        case Operation.SHUTDOWN:
            logging.info("Shutdown requested")
            return True


def run_loop(
    config: Config, db: Database, should_shutdown: Callable[[], bool], pipepath: Path
) -> int:
    """Main event loop."""
    parser = Parser(MAXLINESIZE * 2)
    sel = selectors.DefaultSelector()

    def open_pipe() -> bool:
        try:
            fd = os.open(pipepath, os.O_RDONLY | os.O_NONBLOCK)
            sel.register(fd, selectors.EVENT_READ)
            parser.reset()
            return True
        except OSError as e:
            logging.error("Failed to open command pipe: %s", e)
            return False

    if not open_pipe():
        return -1

    try:
        while True:
            try:
                events = sel.select(timeout=1.0)
            except InterruptedError:
                continue

            for key, mask in events:
                if mask & selectors.EVENT_READ:
                    nread = 0
                    try:
                        nread = parser.input(key.fd)
                    except BufferError:
                        logging.error("Parser buffer full")
                        continue
                    except OSError as e:
                        logging.error("Read error: %s", e)
                        nread = 0

                    if nread == 0:
                        logging.debug("Client disconnected, reopening pipe")
                        sel.unregister(key.fd)
                        os.close(key.fd)
                        if not open_pipe():
                            return -1
                    else:
                        while (cmd := parser.parse_command()) is not None:
                            if handle_command(config, db, cmd):
                                return 0

            if should_shutdown():
                break
    finally:
        for key in list(sel.get_map().values()):
            os.close(key.fd)
        sel.close()

    return 0


def run_daemon(config: Config) -> int:
    """Run the malachi daemon."""
    signal_received = None
    shutdown_flag = False

    def signal_handler(signum, _frame):
        nonlocal signal_received, shutdown_flag
        signal_received = signum
        shutdown_flag = True

    def should_shutdown():
        """Check if shutdown was requested."""
        return shutdown_flag

    try:
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)
    except ValueError:
        pass

    config.runtimedir.mkdir(parents=True, exist_ok=True, mode=0o700)

    pipepath = config.runtimedir / "command"

    try:
        os.mkfifo(pipepath, 0o622)
    except OSError as e:
        logging.error("Failed to mkfifo: %s", e)
        return -1

    logging.info("Starting daemon")
    logging.info("Command pipe: %s", pipepath)

    with Database(config) as db:
        try:
            rc = run_loop(config, db, should_shutdown, pipepath)
        finally:
            pipepath.unlink(missing_ok=True)

    if signal_received == signal.SIGINT:
        logging.info("Received SIGINT, shutting down")
    elif signal_received == signal.SIGTERM:
        logging.info("Received SIGTERM, shutting down")
    else:
        logging.info("Shutting down")

    return rc


def print_version():
    """Print version information."""
    print(f"malachi={__version__}")
    print(f"python={platform.python_version()}")
    print(f"sqlite={sqlite3.sqlite_version}")


def print_config(config: Config):
    """Print configuration."""
    print(f"platform={platform.system()}")
    print(f"configdir={config.configdir}")
    print(f"datadir={config.datadir}")
    print(f"cachedir={config.cachedir}")
    print(f"runtimedir={config.runtimedir}")


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="Malachi git indexer daemon")
    parser.add_argument("-v", "--version", action="store_true", help="Print version")
    parser.add_argument(
        "-d", "--debug", action="store_true", help="Enable debug logging"
    )
    parser.add_argument(
        "-c", "--config", action="store_true", help="Print configuration"
    )

    args = parser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(
        level=log_level,
        format="%(levelname)s: %(message)s",
    )

    if args.version:
        print_version()
        return 0

    config = Config.from_platformdirs()

    if args.config:
        print_config(config)
        return 0

    if args.debug:
        logging.debug("Debug logging enabled")

    return run_daemon(config)


if __name__ == "__main__":
    sys.exit(main())

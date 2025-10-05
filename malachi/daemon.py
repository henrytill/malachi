import argparse
import logging
import os
import platform
import select
import signal
import sqlite3
import sys
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Optional

from malachi import __version__
from malachi.config import Config
from malachi.database import Database
from malachi.status import writestatus

MAXHASHLEN = 65
MAXOPSIZE = 16
MAXFIELDS = 5
MAXRECORDSIZE = (
    MAXOPSIZE + (2 * os.pathconf("/", "PC_PATH_MAX")) + (2 * MAXHASHLEN) + MAXFIELDS
)


class Opcode(Enum):
    """Command operation types."""

    UNKNOWN = 0
    ADDED = 1
    CHANGED = 2
    REMOVED = 3
    SHUTDOWN = 4


@dataclass
class FileOp:
    """File operation fields."""

    root: str
    roothash: str
    leaf: str
    leafhash: str


@dataclass
class Command:
    """IPC command structure."""

    op: Opcode
    fileop: Optional[FileOp] = None


class Parser:
    """ASCII separator protocol parser."""

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

    def _lookup_op(self, name: str) -> Opcode:
        """Lookup operation by name."""
        ops = {
            "added": Opcode.ADDED,
            "changed": Opcode.CHANGED,
            "removed": Opcode.REMOVED,
            "shutdown": Opcode.SHUTDOWN,
        }
        return ops.get(name, Opcode.UNKNOWN)

    def _parse_record(self, record: str) -> Optional[Command]:
        """Parse individual record string."""
        fields = record.split("\x1f")

        if not fields:
            return None

        opname = fields[0]
        opcode = self._lookup_op(opname)

        if opcode == Opcode.UNKNOWN:
            logging.error("Unknown operation: %s", opname)
            return None

        if opcode == Opcode.SHUTDOWN:
            if len(fields) != 1:
                logging.error("shutdown expects 0 data fields, got %d", len(fields) - 1)
                return None
            return Command(op=opcode)

        if opcode in (Opcode.ADDED, Opcode.CHANGED, Opcode.REMOVED):
            if len(fields) != 5:
                logging.error(
                    "%s expects 4 data fields, got %d", opname, len(fields) - 1
                )
                return None

            fileop = FileOp(
                root=fields[1],
                roothash=fields[2],
                leaf=fields[3],
                leafhash=fields[4],
            )
            return Command(op=opcode, fileop=fileop)

        return None

    def parse_command(self, on_generation=None) -> Optional[Command]:
        """Parse next command from buffer. Calls on_generation() when GS found."""
        if not self.buf:
            return None

        gs_pos = self.buf.find(b"\x1d")
        rs_pos = self.buf.find(b"\x1e")

        if gs_pos != -1 and (rs_pos == -1 or gs_pos < rs_pos):
            self.buf = self.buf[gs_pos + 1 :]
            if on_generation:
                on_generation()
            return None

        if rs_pos == -1:
            return None

        record = self.buf[:rs_pos].decode("utf-8")
        self.buf = self.buf[rs_pos + 1 :]

        return self._parse_record(record)


def handle_command(cmd: Command, pending_commands: list) -> bool:
    """Handle parsed command. Returns True to shutdown."""
    match cmd.op:
        case Opcode.ADDED:
            logging.info(
                "Adding leaf: %s in root %s (roothash: %s, leafhash: %s)",
                cmd.fileop.leaf,
                cmd.fileop.root,
                cmd.fileop.roothash,
                cmd.fileop.leafhash,
            )
            pending_commands.append(cmd)
            return False
        case Opcode.CHANGED:
            logging.info(
                "Updating leaf: %s in root %s (roothash: %s, leafhash: %s)",
                cmd.fileop.leaf,
                cmd.fileop.root,
                cmd.fileop.roothash,
                cmd.fileop.leafhash,
            )
            pending_commands.append(cmd)
            return False
        case Opcode.REMOVED:
            logging.info(
                "Removing leaf: %s from root %s (roothash: %s, leafhash: %s)",
                cmd.fileop.leaf,
                cmd.fileop.root,
                cmd.fileop.roothash,
                cmd.fileop.leafhash,
            )
            pending_commands.append(cmd)
            return False
        case Opcode.SHUTDOWN:
            logging.info("Shutdown requested")
            return True
        case _:
            logging.error("Unknown operation")
            return False


def read_commands(
    pipefd: int, parser: Parser, pending_commands: list, on_generation
) -> bool:
    """Read and process commands. Returns True to shutdown."""
    try:
        nread = parser.input(pipefd)
        if nread == 0:
            return False
    except BufferError:
        logging.error("Parser buffer full, processing pending commands")
    except OSError as e:
        logging.error("Read error: %s", e)
        return False

    while (cmd := parser.parse_command(on_generation)) is not None:
        if handle_command(cmd, pending_commands):
            return True

    return False


def run_loop(pipepath: Path, should_shutdown, db: Database, config: Config) -> int:
    """Main event loop."""
    parser = Parser(MAXRECORDSIZE * 2)
    pending_commands = []

    def commit_pending():
        """Commit all pending commands on generation boundary."""
        if not pending_commands:
            return

        # Extract unique roots and their final roothash (last command wins)
        roots = {}
        for cmd in pending_commands:
            if cmd.fileop:
                roots[cmd.fileop.root] = cmd.fileop.roothash

        # Batch update database and status files
        for root, roothash in roots.items():
            db.setrepohash(root, roothash)
            writestatus(config.runtimedir, root, roothash)

        pending_commands.clear()

    poll_obj = select.poll()
    pipefd = None
    shutdown_requested = False

    def open_pipe():
        fd = os.open(pipepath, os.O_RDONLY | os.O_NONBLOCK)
        poll_obj.register(fd, select.POLLIN)
        parser.reset()
        return fd

    try:
        pipefd = open_pipe()
    except OSError as e:
        logging.error("Failed to open command pipe: %s", e)
        return -1

    while not shutdown_requested and not should_shutdown():
        try:
            events = poll_obj.poll(1000)
        except InterruptedError:
            continue

        for fd, event in events:
            if event & select.POLLERR:
                logging.error("Pipe error occurred")
                if pipefd is not None:
                    poll_obj.unregister(pipefd)
                    os.close(pipefd)
                return -1

            if event & select.POLLIN:
                if shutdown_requested := read_commands(
                    fd, parser, pending_commands, commit_pending
                ):
                    break

            if event & select.POLLHUP:
                logging.debug("Client disconnected, reopening pipe")
                poll_obj.unregister(pipefd)
                os.close(pipefd)
                try:
                    pipefd = open_pipe()
                except OSError as e:
                    logging.error("Failed to reopen pipe: %s", e)
                    return -1

    if pipefd is not None:
        poll_obj.unregister(pipefd)
        os.close(pipefd)

    commit_pending()

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
            rc = run_loop(pipepath, should_shutdown, db, config)
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

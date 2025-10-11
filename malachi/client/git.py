import argparse
import json
import logging
import os
import shutil
import stat
import subprocess
import sys
import tempfile

git = shutil.which("git")

logger = logging.getLogger(__name__)
handler = logging.StreamHandler(sys.stderr)
handler.setFormatter(logging.Formatter("[%(levelname)s] %(message)s"))
logger.addHandler(handler)


def getrepo(env):
    repo = env.get("GIT_WORK_TREE", os.getcwd())
    result = subprocess.run(
        [git, "-C", repo, "rev-parse", "--show-toplevel"],
        capture_output=True,
        text=True,
        check=True,
    )
    return result.stdout.strip()


def getruntimedir(env):
    uid = os.getuid()
    runtimedir = env.get("XDG_RUNTIME_DIR", os.path.join("/", "run", "user", str(uid)))
    if os.path.isdir(runtimedir) and os.access(runtimedir, os.W_OK):
        return os.path.join(runtimedir, "malachi")
    return os.path.join(tempfile.gettempdir(), f"malachi-{uid}")


def crawl(env):
    repo = getrepo(env)
    logger.debug("repo=%s", repo)

    runtimedir = getruntimedir(env)
    commandpipe = os.path.join(runtimedir, "command")

    logger.debug("runtimedir=%s", runtimedir)
    logger.debug("commandpipe=%s", commandpipe)

    if not os.path.exists(commandpipe) or not stat.S_ISFIFO(
        os.stat(commandpipe).st_mode
    ):
        logger.error("daemon pipe not found at %s", commandpipe)
        return

    command = {"op": "add", "path": repo}
    with open(commandpipe, "w", encoding="utf-8") as fh:
        json.dump(command, fh)
        fh.write("\n")
        fh.flush()

    logger.debug("sent add command for %s", repo)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", action="store_true", help="Enable debug output")
    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.d else logging.WARNING)

    crawl(os.environ)


if __name__ == "__main__":
    main()

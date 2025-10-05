import argparse
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


def gethead(repo):
    result = subprocess.run(
        [git, "-C", repo, "rev-parse", "HEAD"],
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


def firstrun(repo, head, fh=sys.stdout):
    with subprocess.Popen(
        [git, "-C", repo, "ls-tree", "-r", "--format=%(path) %(objectname)", head],
        stdout=subprocess.PIPE,
        text=True,
    ) as proc:
        for line in proc.stdout:
            path, objhash = line.rstrip("\n").split(" ", 1)
            fh.write(f"added\x1f{repo}\x1f{head}\x1f{path}\x1f{objhash}\x1e")

        fh.write("\x1d")
        fh.flush()

        proc.wait()
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, proc.args)


def genevents(repo, cached, head, fh=sys.stdout):
    with subprocess.Popen(
        [git, "-C", repo, "diff-tree", "--raw", cached, head],
        stdout=subprocess.PIPE,
        text=True,
    ) as proc:
        for line in proc.stdout:
            fields = line.rstrip("\n").split("\t", 1)
            if len(fields) != 2:
                continue

            info, path = fields
            parts = info.split()
            if len(parts) < 5:
                continue

            status = parts[4]
            if status == "A":
                op = "added"
            elif status == "M":
                op = "changed"
            elif status == "D":
                op = "removed"
            else:
                continue

            objhash = parts[2] if status == "D" else parts[3]
            fh.write(f"{op}\x1f{repo}\x1f{head}\x1f{path}\x1f{objhash}\x1e")

        fh.write("\x1d")
        fh.flush()

        proc.wait()
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, proc.args)


def crawl(env):
    repo = getrepo(env)
    logger.debug("repo=%s", repo)

    head = gethead(repo)
    logger.debug("head=%s", head)

    runtimedir = getruntimedir(env)
    cachefile = os.path.join(runtimedir, "roots" + repo)
    commandpipe = os.path.join(runtimedir, "command")

    logger.debug("runtimedir=%s", runtimedir)
    logger.debug("cachefile=%s", cachefile)
    logger.debug("commandpipe=%s", commandpipe)

    if not os.path.exists(cachefile):
        logger.debug("first run - generating add events for all files")

        if os.path.exists(commandpipe) and stat.S_ISFIFO(os.stat(commandpipe).st_mode):
            with open(commandpipe, "w", encoding="utf-8") as fh:
                firstrun(repo, head, fh)
        else:
            logger.debug("daemon pipe not found, writing to stdout instead")
            firstrun(repo, head)

        return

    with open(cachefile, encoding="utf-8") as fh:
        cached = fh.readline().strip()

    if cached == head:
        logger.debug("no changes since last run")
        return

    if os.path.exists(commandpipe) and stat.S_ISFIFO(os.stat(commandpipe).st_mode):
        with open(commandpipe, "w", encoding="utf-8") as fh:
            genevents(repo, cached, head, fh)
    else:
        logger.debug("daemon pipe not found, writing to stdout instead")
        genevents(repo, cached, head)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", action="store_true", help="Enable debug output")
    args = parser.parse_args()

    logger.setLevel(logging.DEBUG if args.d else logging.WARNING)

    crawl(os.environ)


if __name__ == "__main__":
    main()

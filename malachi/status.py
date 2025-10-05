from pathlib import Path


def ensurestatusdir(runtimedir: Path, repopath: str):
    statusbase = runtimedir / "roots"
    statusfile = statusbase / repopath.lstrip("/")
    statusdir = statusfile.parent
    statusdir.mkdir(parents=True, exist_ok=True, mode=0o700)


def writestatus(runtimedir: Path, repopath: str, sha: str):
    ensurestatusdir(runtimedir, repopath)
    statusbase = runtimedir / "roots"
    statusfile = statusbase / repopath.lstrip("/")
    statusfile.write_text(f"{sha}\n")

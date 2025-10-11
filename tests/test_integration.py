# pylint: disable=duplicate-code
import json
import os
import subprocess
import tempfile
import threading
import time
import unittest
from pathlib import Path

from malachi.client.git import crawl
from malachi.config import Config
from malachi.daemon import run_daemon
from malachi.database import Database


class TestIntegration(unittest.TestCase):
    def setUp(self):
        # pylint: disable=consider-using-with
        self.tmpdir = tempfile.TemporaryDirectory()
        self.config = Config(
            configdir=Path(self.tmpdir.name) / "config",
            datadir=Path(self.tmpdir.name) / "data",
            cachedir=Path(self.tmpdir.name) / "cache",
            runtimedir=Path(self.tmpdir.name) / "malachi",
        )
        self.daemon_thread = None
        self.daemon_started = threading.Event()

    def tearDown(self):
        if self.daemon_thread and self.daemon_thread.is_alive():
            pipepath = self.config.runtimedir / "command"
            if pipepath.exists():
                with open(pipepath, "w", encoding="utf-8") as f:
                    json.dump({"op": "shutdown"}, f)
                    f.write("\n")
            self.daemon_thread.join(timeout=2)
        self.tmpdir.cleanup()

    def start_daemon(self):
        def run():
            run_daemon(self.config)

        self.daemon_thread = threading.Thread(target=run, daemon=True)
        self.daemon_thread.start()

        pipepath = self.config.runtimedir / "command"
        for _ in range(50):
            if pipepath.exists() and os.access(pipepath, os.W_OK):
                self.daemon_started.set()
                return
            time.sleep(0.1)
        self.fail("Daemon failed to start")

    def send_command(self, command: dict):
        pipepath = self.config.runtimedir / "command"
        with open(pipepath, "w", encoding="utf-8") as f:
            json.dump(command, f)
            f.write("\n")

    def wait_for_status(
        self, repo_path: Path, expected_hash: str, timeout: float = 5.0
    ):
        """Wait for status file to contain expected hash."""
        statusfile = self.config.runtimedir / "roots" / repo_path.relative_to("/")
        start = time.time()
        while time.time() - start < timeout:
            if statusfile.exists():
                content = statusfile.read_text().strip()
                if content == expected_hash:
                    return
            time.sleep(0.05)
        self.fail(
            f"Timeout waiting for {repo_path} status. "
            f"Expected: {expected_hash}, "
            f"File exists: {statusfile.exists()}"
        )

    def test_daemon_starts_and_stops(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        pipepath = self.config.runtimedir / "command"
        self.assertTrue(pipepath.exists())

        self.send_command({"op": "shutdown"})
        self.daemon_thread.join(timeout=2)
        self.assertFalse(self.daemon_thread.is_alive())

    def create_git_repo(self, path: Path, files: dict):
        """Create a git repo with specified files."""
        path.mkdir(parents=True, exist_ok=True)
        subprocess.run(["git", "init"], cwd=path, check=True, capture_output=True)
        subprocess.run(
            ["git", "config", "user.email", "test@example.com"],
            cwd=path,
            check=True,
            capture_output=True,
        )
        subprocess.run(
            ["git", "config", "user.name", "Test User"],
            cwd=path,
            check=True,
            capture_output=True,
        )

        for filename, content in files.items():
            filepath = path / filename
            filepath.parent.mkdir(parents=True, exist_ok=True)
            filepath.write_text(content)

        subprocess.run(["git", "add", "."], cwd=path, check=True, capture_output=True)
        subprocess.run(
            ["git", "commit", "-m", "Initial commit"],
            cwd=path,
            check=True,
            capture_output=True,
        )

        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=path,
            check=True,
            capture_output=True,
            text=True,
        )
        return result.stdout.strip()

    def test_add_repo_with_git(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        repo_path = Path(self.tmpdir.name) / "test_repo"
        head_sha = self.create_git_repo(
            repo_path, {"README.md": "# Test", "src/main.py": "print('hello')"}
        )

        self.send_command({"op": "add", "path": str(repo_path)})
        self.wait_for_status(repo_path, head_sha)

        with Database(self.config) as db:
            stored_sha = db.get_repo_hash(repo_path)
            self.assertEqual(stored_sha, head_sha)

            root_id = db.get_repo_id(repo_path)
            self.assertIsNotNone(root_id)

            cursor = db.conn.execute(
                "SELECT leaf_path FROM leaves WHERE root_id = ? ORDER BY leaf_path",
                (root_id,),
            )
            paths = [row[0] for row in cursor.fetchall()]
            self.assertEqual(paths, ["README.md", "src/main.py"])

        statusfile = self.config.runtimedir / "roots" / repo_path.relative_to("/")
        self.assertTrue(statusfile.exists())
        self.assertEqual(statusfile.read_text().strip(), head_sha)

    def test_add_repo_incremental_update(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        repo_path = Path(self.tmpdir.name) / "test_repo"
        initial_sha = self.create_git_repo(repo_path, {"file1.txt": "initial"})

        self.send_command({"op": "add", "path": str(repo_path)})
        self.wait_for_status(repo_path, initial_sha)

        with Database(self.config) as db:
            root_id = db.get_repo_id(repo_path)
            cursor = db.conn.execute(
                "SELECT leaf_path FROM leaves WHERE root_id = ?",
                (root_id,),
            )
            initial_files = [row[0] for row in cursor.fetchall()]
            self.assertEqual(
                initial_files,
                ["file1.txt"],
                "Initial indexing should have indexed file1.txt",
            )

        (repo_path / "file2.txt").write_text("new file")
        subprocess.run(
            ["git", "add", "file2.txt"], cwd=repo_path, check=True, capture_output=True
        )
        subprocess.run(
            ["git", "commit", "-m", "Add file2"],
            cwd=repo_path,
            check=True,
            capture_output=True,
        )

        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=repo_path,
            check=True,
            capture_output=True,
            text=True,
        )
        new_sha = result.stdout.strip()

        self.send_command({"op": "add", "path": str(repo_path)})
        self.wait_for_status(repo_path, new_sha)

        with Database(self.config) as db:
            stored_sha = db.get_repo_hash(repo_path)
            self.assertEqual(stored_sha, new_sha)
            self.assertNotEqual(stored_sha, initial_sha)

            root_id = db.get_repo_id(repo_path)
            cursor = db.conn.execute(
                "SELECT leaf_path FROM leaves WHERE root_id = ? ORDER BY leaf_path",
                (root_id,),
            )
            paths = [row[0] for row in cursor.fetchall()]
            self.assertEqual(paths, ["file1.txt", "file2.txt"])

    def test_add_repo_already_current(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        repo_path = Path(self.tmpdir.name) / "test_repo"
        head_sha = self.create_git_repo(repo_path, {"file.txt": "content"})

        self.send_command({"op": "add", "path": str(repo_path)})
        self.wait_for_status(repo_path, head_sha)

        with Database(self.config) as db:
            first_sha = db.get_repo_hash(repo_path)

        self.send_command({"op": "add", "path": str(repo_path)})
        self.wait_for_status(repo_path, head_sha)

        with Database(self.config) as db:
            second_sha = db.get_repo_hash(repo_path)

        self.assertEqual(first_sha, second_sha)
        self.assertEqual(first_sha, head_sha)

    def test_git_client_integration(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        repo_path = (Path(self.tmpdir.name) / "test_repo").resolve()
        head_sha = self.create_git_repo(repo_path, {"README.md": "# Test"})

        env = {
            "GIT_WORK_TREE": str(repo_path),
            "XDG_RUNTIME_DIR": self.tmpdir.name,
        }

        crawl(env)
        self.wait_for_status(repo_path, head_sha)

        with Database(self.config) as db:
            stored_sha = db.get_repo_hash(repo_path)
            self.assertEqual(stored_sha, head_sha)

            root_id = db.get_repo_id(repo_path)
            self.assertIsNotNone(root_id)

            cursor = db.conn.execute(
                "SELECT leaf_path FROM leaves WHERE root_id = ?", (root_id,)
            )
            paths = [row[0] for row in cursor.fetchall()]
            self.assertEqual(paths, ["README.md"])


if __name__ == "__main__":
    unittest.main()

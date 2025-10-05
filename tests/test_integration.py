import os
import tempfile
import threading
import time
import unittest
from pathlib import Path

from malachi.config import Config
from malachi.daemon import run_daemon


class TestIntegration(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.config = Config(
            configdir=Path(self.tmpdir.name) / "config",
            datadir=Path(self.tmpdir.name) / "data",
            cachedir=Path(self.tmpdir.name) / "cache",
            runtimedir=Path(self.tmpdir.name) / "runtime",
        )
        self.daemon_thread = None
        self.daemon_started = threading.Event()

    def tearDown(self):
        if self.daemon_thread and self.daemon_thread.is_alive():
            pipepath = self.config.runtimedir / "command"
            if pipepath.exists():
                with open(pipepath, "w", encoding="utf-8") as f:
                    f.write("shutdown\x1e")
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

    def send_command(self, command: str):
        pipepath = self.config.runtimedir / "command"
        with open(pipepath, "w", encoding="utf-8") as f:
            f.write(command)

    def test_daemon_starts_and_stops(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        pipepath = self.config.runtimedir / "command"
        self.assertTrue(pipepath.exists())

        self.send_command("shutdown\x1e")
        self.daemon_thread.join(timeout=2)
        self.assertFalse(self.daemon_thread.is_alive())

    def test_added_command_creates_status_file(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        self.send_command(
            "added\x1f/home/user/project\x1fabc123\x1fREADME.md\x1fdef456\x1e\x1d"
        )
        time.sleep(0.1)

        statusfile = self.config.runtimedir / "roots" / "home/user/project"
        self.assertTrue(statusfile.exists())
        self.assertEqual(statusfile.read_text().strip(), "abc123")

    def test_changed_command_updates_status_file(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        self.send_command(
            "added\x1f/home/user/project\x1fold_sha\x1fREADME.md\x1fdef456\x1e\x1d"
        )
        time.sleep(0.1)

        self.send_command(
            "changed\x1f/home/user/project\x1fnew_sha\x1fREADME.md\x1fdef456\x1e\x1d"
        )
        time.sleep(0.1)

        statusfile = self.config.runtimedir / "roots" / "home/user/project"
        self.assertEqual(statusfile.read_text().strip(), "new_sha")

    def test_database_stores_repo_hash(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        self.send_command(
            "added\x1f/home/user/project\x1ftest_sha\x1fREADME.md\x1fdef456\x1e\x1d"
        )
        time.sleep(0.1)

        from malachi.database import Database

        with Database(self.config) as db:
            sha = db.getrepohash("/home/user/project")
            self.assertEqual(sha, "test_sha")

    def test_multiple_commands_in_sequence(self):
        self.start_daemon()
        self.assertTrue(self.daemon_started.wait(timeout=5))

        commands = [
            "added\x1f/repo1\x1fsha1\x1ffile1.txt\x1fhash1\x1e",
            "added\x1f/repo2\x1fsha2\x1ffile2.txt\x1fhash2\x1e",
            "changed\x1f/repo1\x1fsha1_updated\x1ffile1.txt\x1fhash1_new\x1e",
            "\x1d",  # Generation separator to trigger commit
        ]

        for cmd in commands:
            self.send_command(cmd)
            time.sleep(0.05)

        time.sleep(0.1)

        status1 = self.config.runtimedir / "roots" / "repo1"
        status2 = self.config.runtimedir / "roots" / "repo2"

        self.assertEqual(status1.read_text().strip(), "sha1_updated")
        self.assertEqual(status2.read_text().strip(), "sha2")


if __name__ == "__main__":
    unittest.main()

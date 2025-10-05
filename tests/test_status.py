import tempfile
import unittest
from pathlib import Path

from malachi.status import ensurestatusdir, writestatus


class TestStatus(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.runtimedir = Path(self.tmpdir.name) / "runtime"

    def tearDown(self):
        self.tmpdir.cleanup()

    def test_ensurestatusdir_creates_directory(self):
        repopath = "/home/user/project"
        ensurestatusdir(self.runtimedir, repopath)
        expected_dir = self.runtimedir / "roots" / "home/user/project"
        self.assertTrue(expected_dir.parent.exists())

    def test_ensurestatusdir_handles_nested_paths(self):
        repopath = "/home/user/deep/nested/project"
        ensurestatusdir(self.runtimedir, repopath)
        expected_dir = self.runtimedir / "roots" / "home/user/deep/nested/project"
        self.assertTrue(expected_dir.parent.exists())

    def test_writestatus_creates_file(self):
        repopath = "/home/user/project"
        sha = "abc123def456"
        writestatus(self.runtimedir, repopath, sha)
        statusfile = self.runtimedir / "roots" / "home/user/project"
        self.assertTrue(statusfile.exists())

    def test_writestatus_writes_correct_content(self):
        repopath = "/home/user/project"
        sha = "def456abc789"
        writestatus(self.runtimedir, repopath, sha)
        statusfile = self.runtimedir / "roots" / "home/user/project"
        content = statusfile.read_text()
        self.assertEqual(content.strip(), sha)

    def test_writestatus_overwrites_existing(self):
        repopath = "/home/user/project"
        writestatus(self.runtimedir, repopath, "old_sha")
        writestatus(self.runtimedir, repopath, "new_sha")
        statusfile = self.runtimedir / "roots" / "home/user/project"
        content = statusfile.read_text()
        self.assertEqual(content.strip(), "new_sha")

    def test_writestatus_handles_leading_slash(self):
        repopath = "/home/user/project"
        sha = "test_sha"
        writestatus(self.runtimedir, repopath, sha)
        statusfile = self.runtimedir / "roots" / "home/user/project"
        self.assertTrue(statusfile.exists())
        self.assertEqual(statusfile.read_text().strip(), sha)


if __name__ == "__main__":
    unittest.main()

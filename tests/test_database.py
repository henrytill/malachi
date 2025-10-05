import tempfile
import unittest
from pathlib import Path

from malachi.config import Config
from malachi.database import Database


class TestDatabase(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.TemporaryDirectory()
        self.config = Config(
            configdir=Path(self.tmpdir.name) / "config",
            datadir=Path(self.tmpdir.name) / "data",
            cachedir=Path(self.tmpdir.name) / "cache",
            runtimedir=Path(self.tmpdir.name) / "runtime",
        )

    def tearDown(self):
        self.tmpdir.cleanup()

    def test_context_manager(self):
        with Database(self.config) as db:
            self.assertIsNotNone(db.conn)
        self.assertIsNone(db.conn)

    def test_creates_cache_directory(self):
        with Database(self.config):
            self.assertTrue(self.config.cachedir.exists())

    def test_creates_database_file(self):
        with Database(self.config) as db:
            self.assertTrue(db.dbpath.exists())

    def test_setrepohash_and_getrepohash(self):
        with Database(self.config) as db:
            db.setrepohash("/home/user/project", "abc123def456")
            sha = db.getrepohash("/home/user/project")
            self.assertEqual(sha, "abc123def456")

    def test_getrepohash_nonexistent(self):
        with Database(self.config) as db:
            sha = db.getrepohash("/nonexistent")
            self.assertIsNone(sha)

    def test_setrepohash_updates_existing(self):
        with Database(self.config) as db:
            db.setrepohash("/home/user/project", "old_sha")
            db.setrepohash("/home/user/project", "new_sha")
            sha = db.getrepohash("/home/user/project")
            self.assertEqual(sha, "new_sha")

    def test_schema_creates_tables(self):
        with Database(self.config) as db:
            cursor = db.conn.execute(
                "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"
            )
            tables = [row[0] for row in cursor.fetchall()]
            self.assertIn("roots", tables)
            self.assertIn("leaves", tables)
            self.assertIn("leaf_pages", tables)
            self.assertIn("schema_version", tables)

    def test_schema_creates_fts_tables(self):
        with Database(self.config) as db:
            cursor = db.conn.execute(
                "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE '%_fts%'"
            )
            fts_tables = [row[0] for row in cursor.fetchall()]
            self.assertTrue(any("leaves_fts" in name for name in fts_tables))
            self.assertTrue(any("leaf_pages_fts" in name for name in fts_tables))


if __name__ == "__main__":
    unittest.main()

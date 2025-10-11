import sqlite3
from pathlib import Path
from typing import Optional

from malachi.config import Config


class Database:
    """SQLite database for repository indexing."""

    def __init__(self, config: Config):
        self.config = config
        self.dbpath = config.cachedir / "index.db"
        self.conn: Optional[sqlite3.Connection] = None

    def __enter__(self):
        self.config.cachedir.mkdir(parents=True, exist_ok=True, mode=0o755)
        self.conn = sqlite3.connect(self.dbpath)
        self.ensure_schema()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.conn:
            self.conn.close()
            self.conn = None

    def ensure_schema(self):
        assert self.conn is not None
        schemapath = Path(__file__).parent / "schema.sql"
        schema = schemapath.read_text()
        self.conn.executescript(schema)
        self.conn.commit()

    def get_repo_hash(self, repopath: Path) -> Optional[str]:
        assert self.conn is not None
        cursor = self.conn.execute(
            "SELECT root_hash FROM roots WHERE root_path = ?", (str(repopath),)
        )
        row = cursor.fetchone()
        return row[0] if row else None

    def set_repo_hash(self, repopath: Path, sha: str):
        assert self.conn is not None
        self.conn.execute(
            "INSERT INTO roots (root_path, root_hash, updated_at) "
            "VALUES (?, ?, CURRENT_TIMESTAMP) "
            "ON CONFLICT(root_path) DO UPDATE SET "
            "root_hash = excluded.root_hash, updated_at = CURRENT_TIMESTAMP",
            (str(repopath), sha),
        )
        self.conn.commit()

    def get_repo_id(self, repopath: Path) -> Optional[int]:
        assert self.conn is not None
        cursor = self.conn.execute(
            "SELECT id FROM roots WHERE root_path = ?", (str(repopath),)
        )
        row = cursor.fetchone()
        return row[0] if row else None

    def add_leaf(self, root_id: int, path: str, leaf_hash: str, size: int = 0):
        assert self.conn is not None
        self.conn.execute(
            "INSERT OR IGNORE INTO leaves "
            "(root_id, leaf_path, leaf_hash, leaf_size) "
            "VALUES (?, ?, ?, ?)",
            (root_id, path, leaf_hash, size),
        )
        self.conn.commit()

    def update_leaf(self, root_id: int, path: str, leaf_hash: str, size: int = 0):
        assert self.conn is not None
        self.conn.execute(
            "UPDATE leaves SET leaf_hash = ?, leaf_size = ? "
            "WHERE root_id = ? AND leaf_path = ?",
            (leaf_hash, size, root_id, path),
        )
        self.conn.commit()

    def remove_leaf(self, root_id: int, path: str):
        assert self.conn is not None
        self.conn.execute(
            "DELETE FROM leaves WHERE root_id = ? AND leaf_path = ?",
            (root_id, path),
        )
        self.conn.commit()

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
        self.ensureschema()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.conn:
            self.conn.close()
            self.conn = None

    def ensureschema(self):
        schemapath = Path(__file__).parent / "schema.sql"
        schema = schemapath.read_text()
        self.conn.executescript(schema)
        self.conn.commit()

    def getrepohash(self, repopath: str) -> Optional[str]:
        cursor = self.conn.execute(
            "SELECT root_hash FROM roots WHERE root_path = ?", (repopath,)
        )
        row = cursor.fetchone()
        return row[0] if row else None

    def setrepohash(self, repopath: str, sha: str):
        self.conn.execute(
            "INSERT OR REPLACE INTO roots (root_path, root_hash, updated_at) "
            "VALUES (?, ?, CURRENT_TIMESTAMP)",
            (repopath, sha),
        )
        self.conn.commit()

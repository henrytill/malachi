CREATE TABLE IF NOT EXISTS schema_version (
    major INTEGER NOT NULL,
    minor INTEGER NOT NULL,
    patch INTEGER NOT NULL,
    applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS roots (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    root_path TEXT NOT NULL UNIQUE,
    root_hash TEXT NOT NULL,
    indexed_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS leaves (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    root_id INTEGER NOT NULL,
    leaf_hash TEXT NOT NULL,
    leaf_path TEXT NOT NULL,
    leaf_size INTEGER NOT NULL,
    mime_type TEXT,
    filter_name TEXT,
    content TEXT,
    indexed_at DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (root_id) REFERENCES roots(id) ON DELETE CASCADE,
    UNIQUE(root_id, leaf_hash, leaf_path)
);

CREATE TABLE IF NOT EXISTS leaf_pages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    leaf_id INTEGER NOT NULL,
    page_number INTEGER NOT NULL,
    content TEXT NOT NULL,

    FOREIGN KEY (leaf_id) REFERENCES leaves(id) ON DELETE CASCADE,
    UNIQUE(leaf_id, page_number)
);

CREATE VIRTUAL TABLE IF NOT EXISTS leaves_fts USING fts5(
    leaf_path,
    content,
    content=leaves,
    content_rowid=id
);

CREATE VIRTUAL TABLE IF NOT EXISTS leaf_pages_fts USING fts5(
    content,
    content=leaf_pages,
    content_rowid=id
);

CREATE TRIGGER IF NOT EXISTS leaves_ai
    AFTER INSERT ON leaves
    BEGIN
        INSERT INTO leaves_fts (rowid, leaf_path, content)
        VALUES (new.id, new.leaf_path, new.content);
    END;

CREATE TRIGGER IF NOT EXISTS leaves_au
    AFTER UPDATE ON leaves
    BEGIN
        UPDATE leaves_fts
           SET leaf_path = new.leaf_path,
               content = new.content
         WHERE rowid = old.id;
    END;

CREATE TRIGGER IF NOT EXISTS leaves_ad
    AFTER DELETE ON leaves
    BEGIN
        DELETE FROM leaves_fts
         WHERE rowid = old.id;
    END;

CREATE TRIGGER IF NOT EXISTS leaf_pages_ai
    AFTER INSERT ON leaf_pages
    BEGIN
        INSERT INTO leaf_pages_fts (rowid, content)
        VALUES (new.id, new.content);
    END;

CREATE TRIGGER IF NOT EXISTS leaf_pages_au
    AFTER UPDATE ON leaf_pages
    BEGIN
        UPDATE leaf_pages_fts
           SET content = new.content
         WHERE rowid = old.id;
    END;

CREATE TRIGGER IF NOT EXISTS leaf_pages_ad
    AFTER DELETE ON leaf_pages
    BEGIN
        DELETE FROM leaf_pages_fts
         WHERE rowid = old.id;
    END;

CREATE INDEX IF NOT EXISTS idx_leaves_root_hash
    ON leaves(root_id, leaf_hash);

CREATE INDEX IF NOT EXISTS idx_leaves_path
    ON leaves(root_id, leaf_path);

CREATE INDEX IF NOT EXISTS idx_roots_path
    ON roots(root_path);

CREATE INDEX IF NOT EXISTS idx_leaf_pages_leaf
    ON leaf_pages(leaf_id, page_number);

INSERT OR IGNORE INTO schema_version (major, minor, patch) VALUES (0, 1, 0);

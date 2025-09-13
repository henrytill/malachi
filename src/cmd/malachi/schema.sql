CREATE TABLE IF NOT EXISTS schema_version (
    major INTEGER NOT NULL,
    minor INTEGER NOT NULL,
    patch INTEGER NOT NULL,
    applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS repositories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL UNIQUE,
    head_sha TEXT NOT NULL,
    indexed_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS file_objects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    repo_id INTEGER NOT NULL,
    object_hash TEXT NOT NULL,
    file_path TEXT NOT NULL,
    file_size INTEGER NOT NULL,
    mime_type TEXT,
    filter_name TEXT,
    content TEXT,
    indexed_at DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (repo_id) REFERENCES repositories(id) ON DELETE CASCADE,
    UNIQUE(repo_id, object_hash, file_path)
);

CREATE TABLE IF NOT EXISTS file_pages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_object_id INTEGER NOT NULL,
    page_number INTEGER NOT NULL,
    content TEXT NOT NULL,

    FOREIGN KEY (file_object_id) REFERENCES file_objects(id) ON DELETE CASCADE,
    UNIQUE(file_object_id, page_number)
);

CREATE VIRTUAL TABLE IF NOT EXISTS file_objects_fts USING fts5(
    file_path,
    content,
    content=file_objects,
    content_rowid=id
);

CREATE VIRTUAL TABLE IF NOT EXISTS file_pages_fts USING fts5(
    content,
    content=file_pages,
    content_rowid=id
);

CREATE TRIGGER IF NOT EXISTS file_objects_ai AFTER INSERT ON file_objects
BEGIN
    INSERT INTO file_objects_fts (rowid, file_path, content)
    VALUES (new.id, new.file_path, new.content);
END;

CREATE TRIGGER IF NOT EXISTS file_objects_au AFTER UPDATE ON file_objects
BEGIN
    UPDATE file_objects_fts SET
        file_path = new.file_path,
        content = new.content
    WHERE rowid = old.id;
END;

CREATE TRIGGER IF NOT EXISTS file_objects_ad AFTER DELETE ON file_objects
BEGIN
    DELETE FROM file_objects_fts WHERE rowid = old.id;
END;

CREATE TRIGGER IF NOT EXISTS file_pages_ai AFTER INSERT ON file_pages
BEGIN
    INSERT INTO file_pages_fts (rowid, content)
    VALUES (new.id, new.content);
END;

CREATE TRIGGER IF NOT EXISTS file_pages_au AFTER UPDATE ON file_pages
BEGIN
    UPDATE file_pages_fts SET
        content = new.content
    WHERE rowid = old.id;
END;

CREATE TRIGGER IF NOT EXISTS file_pages_ad AFTER DELETE ON file_pages
BEGIN
    DELETE FROM file_pages_fts WHERE rowid = old.id;
END;

CREATE INDEX IF NOT EXISTS idx_file_objects_repo_hash
    ON file_objects(repo_id, object_hash);

CREATE INDEX IF NOT EXISTS idx_file_objects_path
    ON file_objects(repo_id, file_path);

CREATE INDEX IF NOT EXISTS idx_repositories_path
    ON repositories(path);

CREATE INDEX IF NOT EXISTS idx_file_pages_file_object
    ON file_pages(file_object_id, page_number);

INSERT OR IGNORE INTO schema_version (major, minor, patch) VALUES (0, 1, 0);

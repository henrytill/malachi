#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "malachi.h"
#include "schema.h"

struct Database {
	sqlite3 *conn;
	char *path;
};

Database *
dbcreate(Config const *config, Error *err)
{
	Database *db = malloc(sizeof(Database));
	if(!db) {
		err->rc = ENOMEM;
		err->msg = "Failed to allocate database";
		return NULL;
	}

	db->conn = NULL;
	db->path = NULL;

	int rc = mkdirp(config->cachedir, 0755);
	if(rc != 0) {
		err->rc = errno;
		err->msg = "Failed to create cache directory";
		free(db);
		return NULL;
	}

	char *dbpath = joinpath2(config->cachedir, "index.db");
	if(!dbpath) {
		err->rc = ENOMEM;
		err->msg = "Failed to allocate database path";
		free(db);
		return NULL;
	}

	rc = sqlite3_open(dbpath, &db->conn);
	if(rc != SQLITE_OK) {
		err->rc = rc;
		err->msg = sqlite3_errmsg(db->conn);
		free(dbpath);
		free(db);
		return NULL;
	}

	db->path = dbpath;

	rc = dbensure(db, err);
	if(rc != 0) {
		dbdestroy(db);
		return NULL;
	}

	return db;
}

void
dbdestroy(Database *db)
{
	if(!db)
		return;

	if(db->conn)
		sqlite3_close(db->conn);

	if(db->path)
		free(db->path);

	free(db);
}

int
dbensure(Database *db, Error *err)
{
	char const *sql = MALACHI_SCHEMA_SQL;
	int rc = sqlite3_exec(db->conn, sql, NULL, NULL, NULL);

	if(rc != SQLITE_OK) {
		err->rc = rc;
		err->msg = sqlite3_errmsg(db->conn);
		return -1;
	}

	return 0;
}

char *
dbrepoget(Database *db, char const *repopath)
{
	char const *sql = "SELECT head_sha FROM repositories WHERE path = ?";
	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db->conn, sql, -1, &stmt, NULL);
	if(rc != SQLITE_OK) {
		logerror("Failed to prepare repo query: %s", sqlite3_errmsg(db->conn));
		return NULL;
	}

	rc = sqlite3_bind_text(stmt, 1, repopath, -1, SQLITE_STATIC);
	if(rc != SQLITE_OK) {
		logerror("Failed to bind repo path: %s", sqlite3_errmsg(db->conn));
		sqlite3_finalize(stmt);
		return NULL;
	}

	char *sha = NULL;
	rc = sqlite3_step(stmt);
	if(rc == SQLITE_ROW) {
		char const *result = (char const *)sqlite3_column_text(stmt, 0);
		if(result) {
			size_t result_len = strlen(result);
			sha = malloc(result_len + 1);
			if(sha)
				memcpy(sha, result, result_len + 1);
		}
	} else if(rc != SQLITE_DONE) {
		logerror("Failed to execute repo query: %s", sqlite3_errmsg(db->conn));
	}

	sqlite3_finalize(stmt);
	return sha;
}

int
dbreposet(Database *db, char const *repopath, char const *sha)
{
	char const *sql = "INSERT OR REPLACE INTO repositories (path, head_sha, updated_at) VALUES (?, ?, CURRENT_TIMESTAMP)";
	sqlite3_stmt *stmt;

	int rc = sqlite3_prepare_v2(db->conn, sql, -1, &stmt, NULL);
	if(rc != SQLITE_OK) {
		logerror("Failed to prepare repo update: %s", sqlite3_errmsg(db->conn));
		return -1;
	}

	rc = sqlite3_bind_text(stmt, 1, repopath, -1, SQLITE_STATIC);
	if(rc != SQLITE_OK) {
		logerror("Failed to bind repo path: %s", sqlite3_errmsg(db->conn));
		sqlite3_finalize(stmt);
		return -1;
	}

	rc = sqlite3_bind_text(stmt, 2, sha, -1, SQLITE_STATIC);
	if(rc != SQLITE_OK) {
		logerror("Failed to bind SHA: %s", sqlite3_errmsg(db->conn));
		sqlite3_finalize(stmt);
		return -1;
	}

	rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	if(rc != SQLITE_DONE) {
		logerror("Failed to update repository: %s", sqlite3_errmsg(db->conn));
		return -1;
	}

	return 0;
}

int
statuswrite(char const *runtimedir, char const *repopath, char const *sha)
{
	if(statusensure(runtimedir, repopath) != 0)
		return -1;

	char *statuspath = joinpath2(runtimedir, "repos");
	if(!statuspath) {
		logerror("Failed to allocate status base path");
		return -1;
	}

	char *fullpath = joinpath2(statuspath, repopath);
	free(statuspath);
	if(!fullpath) {
		logerror("Failed to allocate full status path");
		return -1;
	}

	FILE *fp = fopen(fullpath, "w");
	if(!fp) {
		logerror("Failed to open status file %s: %s", fullpath, strerror(errno));
		free(fullpath);
		return -1;
	}

	(void)fprintf(fp, "%s\n", sha);
	(void)fclose(fp);
	free(fullpath);

	return 0;
}

int
statusensure(char const *runtimedir, char const *repopath)
{
	char *statusbase = joinpath2(runtimedir, "repos");
	if(!statusbase) {
		logerror("Failed to allocate status base path");
		return -1;
	}

	char *statusdir = joinpath2(statusbase, repopath);
	free(statusbase);
	if(!statusdir) {
		logerror("Failed to allocate status directory path");
		return -1;
	}

	char *dirpart = malloc(strlen(statusdir) + 1);
	if(!dirpart) {
		logerror("Failed to allocate directory buffer");
		free(statusdir);
		return -1;
	}

	memcpy(dirpart, statusdir, strlen(statusdir) + 1);
	free(statusdir);

	char *lastslash = strrchr(dirpart, '/');
	if(lastslash && lastslash != dirpart) {
		*lastslash = '\0';

		int rc = mkdirp(dirpart, 0700);
		if(rc != 0) {
			logerror("Failed to create status directory %s: %s", dirpart, strerror(errno));
			free(dirpart);
			return -1;
		}
	}

	free(dirpart);
	return 0;
}

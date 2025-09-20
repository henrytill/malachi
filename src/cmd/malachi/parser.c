#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "malachi.h"

#if __has_attribute(__counted_by__)
#	define COUNTED_BY(member) __attribute__((__counted_by__(member)))
#else
#	define COUNTED_BY(member)
#endif

struct Parser {
	size_t bufsize;
	size_t bufused;
	char buf[] COUNTED_BY(bufsize);
};

Parser *
parsercreate(size_t bufsize)
{
	Parser *p = malloc(sizeof(*p) + (bufsize * sizeof(p->buf[0])));
	if(!p)
		return NULL;

	p->bufsize = bufsize;
	p->bufused = 0;
	return p;
}

void
parserdestroy(Parser *p)
{
	free(p);
}

void
parserreset(Parser *p)
{
	p->bufused = 0;
}

ssize_t
parserinput(Parser *p, int fd)
{
	assert(p->bufused < p->bufsize);

	size_t space = p->bufsize - p->bufused - 1;
	if(space == 0)
		return -Enospace;

	ssize_t nread = read(fd, p->buf + p->bufused, space);
	if(nread <= 0)
		return nread; /* EOF (0) or error (-1) from read() */

	p->bufused += nread;
	assert(p->bufused < p->bufsize);
	p->buf[p->bufused] = '\0';
	return nread;
}

static int
findop(size_t oplen, char const opstr[oplen])
{
	STATIC_ASSERT(NELEM(ops) <= INT_MAX);
	int const nops = (int)NELEM(ops);
	for(int i = 0; i < nops; ++i) {
		if(oplen == ops[i].namelen && strncmp(opstr, ops[i].name, oplen) == 0)
			return i;
	}
	return -1;
}

static int
parserecord(char const *record, Command *cmd) /* NOLINT(readability-function-cognitive-complexity) */
{
	memset(cmd, 0, sizeof(*cmd));

	struct {
		char const *pos;
		size_t len;
	} fields[MAXFIELDS];

	int fieldcount = 0;
	char const *pos = record;

	STATIC_ASSERT(SIZE_MAX >> 1 == LONG_MAX || SIZE_MAX == LONG_MAX);

	for(int i = 0; i < MAXFIELDS && *pos; ++i) {
		fields[i].pos = pos;
		char const *next = strchr(pos, '\x1F');
		fields[i].len = next ? (assert(next >= pos), (size_t)(next - pos)) : strlen(pos);
		pos = next ? next + 1 : pos + fields[i].len;
		fieldcount++;
	}

	if(fieldcount < 1)
		return -1;

	int const opindex = findop(fields[0].len, fields[0].pos);
	if(opindex < 0)
		return -1;

	int const nfields = (assert(ops[opindex].nfields <= INT_MAX), (int)ops[opindex].nfields);
	if(fieldcount != nfields + 1) /* +1 for operation field */
		return -1;

	cmd->op = ops[opindex].op;

	struct Fieldspec const *const fieldspecs = ops[opindex].fields;
	if(fieldspecs == NULL)
		return 0;

	for(int i = 0; i < nfields; ++i) {
		size_t const offset = fieldspecs[i].offset;
		size_t const destsize = fieldspecs[i].size;
		assert(offset + destsize <= sizeof(*cmd));
		char *const dest = (char *)cmd + offset;

		int const sourcelen = (assert(fields[i + 1].len <= INT_MAX), (int)fields[i + 1].len);
		char const *const source = fields[i + 1].pos;

		int const n = snprintf(dest, destsize, "%.*s", sourcelen, source);
		if(n >= (assert(destsize <= INT_MAX), (int)destsize)) {
			char const *const destname = fieldspecs[i].name;
			logerror("%s too long: %d bytes", destname, sourcelen);
			return -1;
		}
	}

	return 0;
}

int
parsecommand(Parser *p, Command *cmd, int *generation)
{
	size_t remaining = 0;
	assert(p->bufused <= p->bufsize);

	if(p->bufused == 0)
		return 0; /* No data */

	char *gs = memchr(p->buf, '\x1D', p->bufused);
	char *rs = memchr(p->buf, '\x1E', p->bufused);
	char *separator = NULL;
	size_t prefixlen = 0;

	/* If GS comes before RS (or RS not found), handle GS */
	if(gs && (!rs || gs < rs)) {
		separator = gs;
		prefixlen = gs - p->buf;
		assert(prefixlen < p->bufused);
		(*generation)++;
		/* No record to parse for GS */
		goto compact;
	}

	if(!rs)
		return 0; /* Incomplete record */

	separator = rs;
	prefixlen = rs - p->buf;
	assert(prefixlen < p->bufused);

	int rc;
	if(prefixlen > MAXRECORDSIZE) {
		logerror("Record too large: %zu bytes, discarding", prefixlen);
		rc = -1;
		goto compact;
	}

	*rs = '\0';
	rc = parserecord(p->buf, cmd);
	if(rc != 0)
		*rs = '\x1E'; /* Restore separator and skip malformed record */

compact:
	/* Move remaining data to start of buffer */
	remaining = p->bufused - (prefixlen + 1);
	memmove(p->buf, separator + 1, remaining);
	p->bufused = remaining;
	assert(p->bufused <= p->bufsize);

	if(separator == gs)
		return 0;
	if(rc == 0)
		return 1;
	return -1;
}

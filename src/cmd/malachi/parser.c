#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static Opcode
parseop(size_t oplen, char const opstr[oplen])
{
	for(size_t i = 0; i < NELEM(ops); ++i) {
		if(oplen == ops[i].namelen && strncmp(opstr, ops[i].name, oplen) == 0)
			return ops[i].op;
	}
	return Opunknown;
}

static int
getnfields(size_t oplen, char const opstr[oplen])
{
	for(size_t i = 0; i < NELEM(ops); ++i) {
		if(oplen == ops[i].namelen && strncmp(opstr, ops[i].name, oplen) == 0) {
			assert(ops[i].nfields <= INT_MAX);
			return (int)ops[i].nfields;
		}
	}
	return -1;
}

static struct Fieldspec const *
getfieldspecs(Opcode op)
{
	for(size_t i = 0; i < NELEM(ops); ++i) {
		if(ops[i].op == op)
			return ops[i].fields;
	}
	return NULL;
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

	int const nfields = getnfields(fields[0].len, fields[0].pos);
	if(nfields < 0 || fieldcount != nfields + 1) /* +1 for operation field */
		return -1;

	cmd->op = parseop(fields[0].len, fields[0].pos);

	struct Fieldspec const *const fieldspecs = getfieldspecs(cmd->op);
	if(fieldspecs) {
		for(int i = 0; i < nfields; ++i) {
			char *const dest = (char *)cmd + fieldspecs[i].offset;
			size_t const destsize = fieldspecs[i].size;
			char const *const destname = fieldspecs[i].name;

			int const sourcelen = (assert(fields[i + 1].len <= INT_MAX), (int)fields[i + 1].len);
			char const *const source = fields[i + 1].pos;

			int const n = snprintf(dest, destsize, "%.*s", sourcelen, source);
			if(n >= (int)destsize) {
				logerror("%s too long: %d bytes", destname, sourcelen);
				return -1;
			}
		}
	}

	return 0;
}

int
parsecommand(Parser *p, Command *cmd)
{
	size_t remaining = 0;
	assert(p->bufused <= p->bufsize);

	if(p->bufused == 0)
		return 0; /* No data */

	char *rs = memchr(p->buf, '\x1E', p->bufused);
	if(!rs)
		return 0; /* Incomplete record */

	size_t recordlen = rs - p->buf;
	assert(recordlen < p->bufused);

	int rc;
	if(recordlen > MAXRECORDSIZE) {
		logerror("Record too large: %zu bytes, discarding", recordlen);
		rc = -1;
		goto compact;
	}

	*rs = '\0';
	rc = parserecord(p->buf, cmd);
	if(rc != 0) {
		/* Restore separator and skip malformed record */
		*rs = '\x1E';
	}

compact:
	/* Move remaining data to start of buffer */
	remaining = p->bufused - (recordlen + 1);
	memmove(p->buf, rs + 1, remaining);
	p->bufused = remaining;
	assert(p->bufused <= p->bufsize);

	return (rc == 0) ? 1 : -1;
}

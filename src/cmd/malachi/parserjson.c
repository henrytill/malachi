#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <yyjson.h>

#include "malachi.h"

#if __has_attribute(__counted_by__)
#    define COUNTED_BY(member) __attribute__((__counted_by__(member)))
#else
#    define COUNTED_BY(member)
#endif

#if __has_attribute(unused)
#    define UNUSED __attribute__((unused))
#else
#    define UNUSED
#endif

enum Parsestate {
    Statelen,
    Statejson,
};

struct Parser {
    size_t bufsize;
    size_t bufused;
    enum Parsestate state;
    uint32_t jsonlen;
    char buf[] COUNTED_BY(bufsize);
};

Parser *parsercreate(size_t bufsize)
{
    Parser *p = malloc(sizeof(*p) + (bufsize * sizeof(p->buf[0])));
    if (p == NULL)
        return NULL;

    p->bufsize = bufsize;
    p->bufused = 0;
    p->state = Statelen;
    p->jsonlen = 0;
    return p;
}

void parserdestroy(Parser *p)
{
    free(p);
}

void parserreset(Parser *p)
{
    p->bufused = 0;
    p->state = Statelen;
    p->jsonlen = 0;
}

ssize_t parserinput(Parser *p, int fd)
{
    assert(p->bufused < p->bufsize);

    size_t space = p->bufsize - p->bufused - 1;
    if (space == 0)
        return -Enospace;

    ssize_t nread = read(fd, p->buf + p->bufused, space);
    if (nread <= 0)
        return nread; /* EOF (0) or error (-1) from read() */

    p->bufused += nread;
    assert(p->bufused < p->bufsize);
    p->buf[p->bufused] = '\0';
    return nread;
}

static int findjsonop(size_t oplen, char const *opstr)
{
    STATIC_ASSERT(NELEM(jsonops) <= INT_MAX);
    int const njsonops = (int)NELEM(jsonops);
    for (int i = 0; i < njsonops; ++i) {
        if (oplen == jsonops[i].namelen && strncmp(opstr, jsonops[i].name, oplen) == 0)
            return i;
    }
    return -1;
}

static int parsejson(char const *jsonstr, size_t jsonlen, Command *cmd)
{
    int ret = -1;
    memset(cmd, 0, sizeof(*cmd));

    yyjson_doc *doc = yyjson_read(jsonstr, jsonlen, 0);
    if (doc == NULL) {
        logerror("Failed to parse JSON");
        return -1;
    }

    yyjson_val *root = yyjson_doc_get_root(doc);
    if (yyjson_is_obj(root) == 0) {
        logerror("JSON root is not an object");
        goto freedoc;
    }

    yyjson_val *opval = yyjson_obj_get(root, "op");
    if (opval == NULL || yyjson_is_str(opval) == 0) {
        logerror("Missing or invalid 'op' field");
        goto freedoc;
    }

    char const *opstr = yyjson_get_str(opval);
    size_t oplen = yyjson_get_len(opval);

    int const opindex = findjsonop(oplen, opstr);
    if (opindex < 0) {
        logerror("Unknown operation: %s", opstr);
        goto freedoc;
    }

    cmd->op = jsonops[opindex].op;

    struct Fieldspec const *const fieldspecs = jsonops[opindex].fields;
    if (fieldspecs == NULL) {
        ret = 0;
        goto freedoc;
    }

    int const nfields = (assert(jsonops[opindex].nfields <= INT_MAX), (int)jsonops[opindex].nfields);

    for (int i = 0; i < nfields; ++i) {
        size_t const offset = fieldspecs[i].offset;
        size_t const destsize = fieldspecs[i].size;
        int const required = fieldspecs[i].required;
        char const *const jsonkey = fieldspecs[i].jsonkey;

        assert(offset + destsize <= sizeof(*cmd));
        char *const dest = (char *)cmd + offset;

        yyjson_val *fieldval = yyjson_obj_get(root, jsonkey);
        if (fieldval == NULL || yyjson_is_str(fieldval) == 0) {
            if (required == 0) {
                continue;
            }
            logerror("Missing or invalid '%s' field for %s operation", jsonkey, opstr);
            goto freedoc;
        }

        char const *fieldstr = yyjson_get_str(fieldval);
        int const n = snprintf(dest, destsize, "%s", fieldstr);
        if (n >= (assert(destsize <= INT_MAX), (int)destsize)) {
            char const *const destname = fieldspecs[i].name;
            logerror("%s too long: %d", destname, strlen(fieldstr));
            goto freedoc;
        }
    }

    ret = 0;

freedoc:
    yyjson_doc_free(doc);
    return ret;
}

int parsecommand(Parser *p, Command *cmd, UNUSED int *generation)
{
    size_t remaining = 0;
    size_t skipbytes = 0;
    int rc = 0;
    assert(p->bufused <= p->bufsize);

    if (p->bufused == 0)
        return 0;

    if (p->state == Statelen) {
        if (p->bufused < sizeof(uint32_t))
            return 0;

        uint32_t len;
        memcpy(&len, p->buf, sizeof(len));
        p->jsonlen = len;

        if (p->jsonlen == 0) {
            logerror("Invalid JSON length: 0, draining buffer");
            skipbytes = p->bufused;
            rc = -1;
            goto compact;
        }

        if (p->jsonlen > p->bufsize - sizeof(uint32_t)) {
            logerror("JSON length too large: %u bytes, draining buffer", p->jsonlen);
            skipbytes = p->bufused;
            p->jsonlen = 0;
            rc = -1;
            goto compact;
        }

        p->state = Statejson;
    }

    if (p->state == Statejson) {
        size_t const totalneeded = sizeof(uint32_t) + p->jsonlen;
        if (p->bufused < totalneeded)
            return 0;

        char *jsonstart = p->buf + sizeof(uint32_t);
        rc = parsejson(jsonstart, p->jsonlen, cmd);

        skipbytes = totalneeded;
        p->state = Statelen;
        p->jsonlen = 0;
        goto compact;
    }

    logerror("Invalid parser state: %d", p->state);
    return -1;

compact:
    /* Move remaining data to start of buffer */
    remaining = p->bufused - skipbytes;
    memmove(p->buf, p->buf + skipbytes, remaining);
    p->bufused = remaining;
    assert(p->bufused <= p->bufsize);

    if (rc == 0)
        return 1;
    return -1;
}

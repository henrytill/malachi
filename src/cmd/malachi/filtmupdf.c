#include <mupdf/fitz.h> /* beware of <mupdf/fitz/system.h>  */
#undef nelem

#include "malachi.h"

static char const *exts[] = {
    ".pdf",
    ".PDF",
    NULL,
};

static int extract(char const *input, char **output)
{
    (void)input;
    *output = NULL;
    return -1; // Not implemented yet
}

static char const *version(void)
{
    return FZ_VERSION;
}

static Filter const mupdf = {
    .name = "mupdf",
    .exts = exts,
    .extract = extract,
    .version = version,
};

__attribute__((constructor)) static void init(void)
{
    filteradd(&mupdf);
}

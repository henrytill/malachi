#include <mupdf/fitz.h>

static const char *const MUPDF_VERSION = FZ_VERSION;

const char *version(void)
{
    return MUPDF_VERSION;
}

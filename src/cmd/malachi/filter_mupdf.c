#include <mupdf/fitz.h>
#include <stdlib.h>
#include <string.h>

#include "dat.h"
#include "fns.h"

static char const *mupdf_extensions[] = {
	".pdf",
	".PDF",
	NULL};

static int
mupdf_extract_text(char const *input_path, char **output_text)
{
	(void)input_path;
	*output_text = NULL;
	return -1; // Not implemented yet
}

static char const *
mupdf_get_version(void)
{
	return FZ_VERSION;
}

static struct filter_ops const mupdf_ops = {
	.name = "mupdf",
	.extensions = mupdf_extensions,
	.extract_text = mupdf_extract_text,
	.get_version = mupdf_get_version,
};

__attribute__((constructor)) static void
filter_mupdf_init(void)
{
	filter_register(&mupdf_ops);
}

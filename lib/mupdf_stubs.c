#include <caml/alloc.h>
#include <caml/memory.h>
#include <mupdf/fitz.h>

CAMLprim value caml_mupdf_version_string(value unit)
{
    CAMLparam1(unit);
    CAMLlocal1(caml_result);
    caml_result = caml_copy_string(FZ_VERSION);
    CAMLreturn(caml_result);
}

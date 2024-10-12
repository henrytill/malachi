#include <caml/alloc.h>
#include <caml/memory.h>
#include <poppler/cpp/poppler-version.h>
#include <string>

extern "C" CAMLprim value caml_poppler_version_string(value unit);

CAMLprim value caml_poppler_version_string(value unit) {
  CAMLparam1(unit);
  std::string result = poppler::version_string();
  CAMLlocal1(caml_result);
  caml_result = caml_copy_string(result.c_str());
  CAMLreturn(caml_result);
}

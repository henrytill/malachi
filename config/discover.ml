module C = Configurator.V1

let call_pkg_config pc =
  let open C.Pkg_config in
  let package = "mupdf" in
  let expr = "mupdf >= 1.19.0" in
  match query_expr_err pc ~package ~expr with
  | Ok c -> c
  | Error err -> C.die "%s" err

let configure c =
  let open C.Pkg_config in
  let { cflags; libs } =
    match get c with
    | None -> { libs = [ "-lmupdf" ]; cflags = [] }
    | Some pc -> call_pkg_config pc
  in
  C.Flags.write_sexp "mupdf_cflags.sexp" cflags;
  C.Flags.write_sexp "mupdf_libs.sexp" libs

let () = C.main ~name:"malachi" configure

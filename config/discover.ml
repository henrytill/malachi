module C = Configurator.V1

let call_pkg_config pc =
  let package = "mupdf" in
  let expr = "mupdf >= 1.19.0" in
  match C.Pkg_config.query_expr_err pc ~package ~expr with
  | Ok c -> c
  | Error err -> C.die "%s" err

let configure c =
  let C.Pkg_config.{ cflags; libs } =
    match C.Pkg_config.get c with
    | None -> { libs = [ "-lmupdf" ]; cflags = [] }
    | Some pc -> call_pkg_config pc
  in
  C.Flags.write_sexp "mupdf_cflags.sexp" cflags;
  C.Flags.write_sexp "mupdf_libs.sexp" libs

let () = C.main ~name:"malachi" configure

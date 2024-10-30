let () =
  let mupdf_version = Mupdf.version () in
  print_endline (Printf.sprintf "mupdf version: %s" mupdf_version)

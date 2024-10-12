let () =
  let poppler_version = Poppler.version () in
  print_endline (Printf.sprintf "poppler version: %s" poppler_version)

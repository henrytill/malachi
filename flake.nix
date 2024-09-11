{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    let
      makeMalachi =
        pkgs:
        pkgs.stdenv.mkDerivation {
          name = "malachi";
          src = builtins.path {
            path = ./.;
            name = "malachi-src";
          };
          nativeBuildInputs = with pkgs; [ pkg-config ];
          buildInputs = with pkgs; [
            libgit2
            mupdf
            sqlite
          ];
          doCheck = true;
          installFlags = [ "DESTDIR=${placeholder "out"}" ];
        };
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.malachi = makeMalachi pkgs;
        packages.default = self.packages.${system}.malachi;
      }
    );
}

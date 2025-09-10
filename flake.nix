{

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  nixConfig = {
    extra-substituters = [ "https://henrytill.cachix.org" ];
    extra-trusted-public-keys = [
      "henrytill.cachix.org-1:EOoUIk8e9627viyFmT6mfqghh/xtfnpzEtqT4jnyn1M="
    ];
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      ...
    }:
    let
      overlay = final: prev: {
        malachi = final.stdenv.mkDerivation {
          name = "malachi";
          src = builtins.path {
            path = ./.;
            name = "malachi-src";
          };
          nativeBuildInputs = with final; [
            meson
            ninja
            pkg-config
          ];
          buildInputs = with final; [
            libgit2
            mupdf
            sqlite
          ];
          preConfigure = ''
            sed -i 's/@MALACHI_COMMIT_SHORT_HASH@/"${self.shortRev or self.dirtyShortRev or ""}"/g' include/project.h.in
          '';
          doCheck = true;
        };
      };
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ overlay ];
        };
      in
      {
        packages.malachi = pkgs.malachi;
        packages.default = self.packages.${system}.malachi;
      }
    );
}

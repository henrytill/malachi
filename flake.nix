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
      makeMalachi =
        pkgs:
        pkgs.stdenv.mkDerivation {
          name = "malachi";
          src = builtins.path {
            path = ./.;
            name = "malachi-src";
          };
          nativeBuildInputs = with pkgs; [
            meson
            ninja
            pkg-config
          ];
          buildInputs = with pkgs; [
            catch2_3
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

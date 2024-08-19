{
  description = "The librarian";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
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
        {
          compiler ? "ghc948",
          doCheck ? true,
        }:
        let
          call = compiler: pkgs.haskell.packages.${compiler}.callCabal2nixWithOptions;
          flags = "";
          src = builtins.path {
            path = ./.;
            name = "malachi-src";
          };
          malachi_ = call compiler "malachi" src flags { };
        in
        pkgs.haskell.lib.overrideCabal malachi_ (_: {
          inherit doCheck;
          isExecutable = true;
          isLibrary = false;
          doHaddock = false;
        });
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.malachi = makeMalachi pkgs { };
        packages.default = self.packages.${system}.malachi;
      }
    );
}

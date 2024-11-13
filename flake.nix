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
      makeAnanke =
        pkgs:
        pkgs.python3Packages.buildPythonApplication {
          name = "malachi";
          pyproject = true;
          build-system = with pkgs.python3Packages; [ setuptools ];
          dependencies = with pkgs.python3Packages; [
            platformdirs
            pygit2
            pymupdf
          ];
          nativeCheckInputs = with pkgs.python3Packages; [ mypy ];
          src = builtins.path {
            path = ./.;
            name = "malachi-src";
          };
          patchPhase = "patchShebangs build.sh";
          preConfigure = "./build.sh generate -g ${self.shortRev or self.dirtyShortRev}";
          checkPhase = "./build.sh test";
        };
    in
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.malachi = makeAnanke pkgs;
        packages.default = self.packages.${system}.malachi;
      }
    );
}

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
      src = builtins.path {
        path = ./.;
        name = "malachi-src";
      };
      overlay = final: prev: {
        malachi = final.python3Packages.buildPythonApplication {
          name = "malachi";
          pyproject = true;
          build-system = with final.python3Packages; [ flit-core ];
          inherit src;
          dependencies = with final.python3Packages; [ platformdirs ];
          patchPhase = "patchShebangs run.py";
          preConfigure = "./run.py generate -g ${self.shortRev or self.dirtyShortRev}";
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
        packages = rec {
          malachi = pkgs.malachi;
          default = malachi;
        };
      }
    );
}

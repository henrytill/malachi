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
            makeWrapper
            meson
            ninja
            pkg-config
          ];
          buildInputs = with final; [
            mupdf
            sqlite
          ];
          preConfigure =
            let
              rev = self.shortRev or self.dirtyShortRev;
            in
            ''
              sed -i 's|@MALACHI_COMMIT_SHORT_HASH@|"${rev}"|g' include/project.h.in
            '';
          doCheck = true;
          postFixup =
            let
              binPath = final.lib.makeBinPath [
                final.git
                final.python3
              ];
            in
            ''
              wrapProgram $out/bin/git-crawl --prefix PATH : "${binPath}"
            '';
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

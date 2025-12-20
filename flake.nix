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
      mkMalachi =
        pkgs:
        pkgs.stdenv.mkDerivation {
          name = "malachi";
          src = self;
          nativeBuildInputs = with pkgs; [
            makeWrapper
            meson
            ninja
            pkg-config
          ];
          buildInputs = with pkgs; [
            mupdf
            sqlite
            yyjson
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
              binPath = pkgs.lib.makeBinPath [ pkgs.git ];
              perlPath = pkgs.perlPackages.makePerlPath [ pkgs.git ];
            in
            ''
              wrapProgram $out/bin/git-crawl \
                --prefix PATH : "${binPath}" \
                --prefix PERL5LIB : "${perlPath}"
            '';
        };

      overlay = final: prev: {
        malachi = mkMalachi final;
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
        devShell = pkgs.mkShell {
          inputsFrom = [ pkgs.malachi ];
          packages = with pkgs; [
            clang-tools
            perlcritic
            perlPackages.PerlTidy
          ];
          hardeningDisable = [ "fortify" ];
        };
      }
    );
}

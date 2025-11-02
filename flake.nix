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
        {
          pkgs,
          jsonProtocol ? true,
        }:
        pkgs.stdenv.mkDerivation {
          name = "malachi" + (if jsonProtocol then "-json" else "-ascii");
          src = self;
          nativeBuildInputs = with pkgs; [
            makeWrapper
            meson
            ninja
            pkg-config
          ];
          buildInputs =
            with pkgs;
            [
              mupdf
              sqlite
            ]
            ++ pkgs.lib.optionals jsonProtocol [ yyjson ];
          preConfigure =
            let
              rev = self.shortRev or self.dirtyShortRev;
            in
            ''
              sed -i 's|@MALACHI_COMMIT_SHORT_HASH@|"${rev}"|g' include/project.h.in
            '';
          mesonFlags = [
            (if jsonProtocol then "-Djson_protocol=true" else "-Djson_protocol=false")
          ];
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
        malachi = mkMalachi {
          pkgs = final;
          jsonProtocol = true;
        };
        malachi-ascii = mkMalachi {
          pkgs = final;
          jsonProtocol = false;
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
        packages.malachi-ascii = pkgs.malachi-ascii;
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

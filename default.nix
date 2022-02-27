{ pkgs ? (import ./nix/nixpkgs-pinned.nix { }) }:

let
  gi = pkgs.nix-gitignore;
  lib = pkgs.lib;

  version = import ./nix/version.nix;
  root = ./.;
  sourceName = "flipdot-gschichtler-source";

  rootSrc = builtins.path {
    path = root;
    name = sourceName;
    filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) root;
  };

in

rec {
  warteraum-static = pkgs.pkgsStatic.callPackage ./nix/warteraum.nix {
    inherit (pkgs.pkgsStatic.llvmPackages) stdenv;
    inherit rootSrc sourceName;
    inherit (python3.pkgs) pytest pytest-randomly requests flipdot-gschichtler;
  };

  warteraum = pkgs.callPackage ./nix/warteraum.nix {
    inherit (pkgs.llvmPackages_latest) stdenv;
    inherit rootSrc sourceName;
    inherit (python3.pkgs) pytest pytest-randomly requests flipdot-gschichtler;
  };

  bahnhofshalle = pkgs.stdenv.mkDerivation {
    pname = "bahnhofshalle";
    inherit version;

    src = rootSrc + "/bahnhofshalle";

    nativeBuildInputs = [
      pkgs.buildPackages.esbuild
    ];

    makeFlags = [
      "DIST=$(out)"
    ];

    installTargets = [ "dist" ];
  };

  anzeigetafel =
    let
      drv = { buildPythonApplication, unifont, flipdots, flipdot-gschichtler }:
        buildPythonApplication {
          pname = "anzeigetafel";
          inherit version;

          src = rootSrc + "/anzeigetafel";

          propagatedBuildInputs = [ flipdots flipdot-gschichtler ];

          doCheck = false;

          patchPhase = ''
            rm flipdots flipdot_gschichtler

            sed -i "s/version = '.*'/version = '${version}'/" setup.py

            sed -i 's|FONT =.*$|FONT = "${unifont}/share/fonts/unifont.pcf.gz"|' anzeigetafel.py
          '';
        };
    in python3.pkgs.callPackage drv { };


  python3 = pkgs.python3.override {
    packageOverrides = self: super: {
      flipdots = self.callPackage ./nix/python-flipdots.nix {
        inherit rootSrc;
      };

      flipdot-gschichtler = self.callPackage ./nix/python-flipdot-gschichtler.nix {
        inherit rootSrc;
      };
    };
  };
}

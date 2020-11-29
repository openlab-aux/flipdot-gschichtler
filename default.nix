{ pkgs ? (import ./nix/nixpkgs-pinned.nix { })
, scryptSalt ? null
, apiTokens ? null
}:

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
  warteraum-static = (pkgs.pkgsStatic.callPackage ./nix/warteraum.nix {
    # todo clang?
    redo = pkgs.pkgsStatic.redo-c;
    inherit scryptSalt apiTokens rootSrc sourceName;
    inherit (python3.pkgs) pytest pytest-randomly requests flipdot-gschichtler;
  }).overrideAttrs (old: {
    # musl, static linking
    postPatch = ''
      cat >> ./build_config << EOF
      CFLAGS="\$CFLAGS -static"
      EOF
    '';
  });

  warteraum = pkgs.callPackage ./nix/warteraum.nix {
    stdenv = pkgs.clangStdenv;
    redo = pkgs.redo-c;
    inherit scryptSalt apiTokens rootSrc sourceName;
    inherit (python3.pkgs) pytest pytest-randomly requests flipdot-gschichtler;
  };

  bahnhofshalle =
    let
      nodePackages = import ./bahnhofshalle/node2nix { inherit pkgs; inherit (pkgs) nodejs; };
      nodeDeps = nodePackages.shell.nodeDependencies;
    in pkgs.stdenv.mkDerivation {
      pname = "bahnhofshalle";
      inherit version;

      src = rootSrc + "/bahnhofshalle";

      buildInputs = [ pkgs.nodejs ];

      buildPhase = ''
        export PARCEL_WORKERS=$NIX_BUILD_CORES
        ln -s ${nodeDeps}/lib/node_modules ./node_modules
        export PATH="${nodeDeps}/bin:$PATH"

        parcel build index.html --out-dir="dist" --no-source-maps

        # fail if parcel doesn't produce an output
        if [[ "$(find dist | wc -l)" -le 1 ]]; then
          exit 1
        fi
      '';

      installPhase = ''
        cp -r dist $out
      '';
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

            sed -i 's|FONT =.*$|FONT = "${unifont}/share/fonts/truetype/unifont.ttf"|' anzeigetafel.py
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

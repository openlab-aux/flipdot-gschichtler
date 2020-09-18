{ pkgs ? (import <nixpkgs> {}) }:

with pkgs;

let
  version = "unstable";
  gi = nix-gitignore;
  root = ./.;
  sourceName = "flipdot-gschichtler-source";

  src = builtins.path {
    path = root;
    name = sourceName;
    filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) root;
  };

  yarn2nix = import (fetchFromGitHub {
    owner  = "sternenseemann";
    repo   = "yarn2nix";
    rev    = "3811ffdc3b43a9ba8b8a7f7316399d8da90f8a38";
    sha256 = "1r5f70qswrxzg0z4pk7v9l59aqnckrp70pxfplhyaq9vvys4f7l7";
  }) { };

  y2nlib = yarn2nix.passthru.nixLib;

  warteraumDrv = { stdenv, redo, scrypt }:
    stdenv.mkDerivation rec {
      pname = "warteraum";
      sourceRoot = sourceName + "/warteraum";

      inherit src version;

      # make whole source tree writeable for redo
      postUnpack = ''
        chmod -R u+w "$sourceRoot/.."
      '';

      buildPhase = "redo";

      doCheck = true;
      checkPhase = "./test/test_queue";

      installPhase = ''
        install -Dm755 warteraum -t $out/bin
        install -Dm755 hashtoken -t $out/bin
      '';

      nativeBuildInputs = [ redo ];

      buildInputs = [ scrypt ];
    };
in

rec {
  warteraum-static = (pkgsStatic.callPackage warteraumDrv {
    # todo clang?
    redo = pkgsStatic.redo-sh;
  }).overrideAttrs (old: {
    # musl, static linking
    patchPhase = ''
      cat >> ./build_config << EOF
      CFLAGS="\$CFLAGS -static"
      EOF
    '';
  });

  warteraum = callPackage warteraumDrv {
    stdenv = clangStdenv;
    redo = redo-sh;
  };

  bahnhofshalle =
    let
      deps = y2nlib.buildNodeDeps
        (y2nlib.callYarnLock ./bahnhofshalle/yarn.lock { });
      tpl = (y2nlib.callPackageJson ./bahnhofshalle/package.json { }) deps;
      node_modules = y2nlib.linkNodeDeps {
        dependencies = tpl.nodeBuildInputs;
        inherit (tpl.key) name;
      };
    in stdenv.mkDerivation {
      inherit (tpl) version;
      inherit (tpl.key) name;

      inherit src;
      sourceRoot = sourceName + "/bahnhofshalle";

      buildInputs = [ redo-sh ];

      buildPhase = ''
        # yeah … idk … node
        ln -s "${node_modules}" node_modules
        export NODE_PATH=node_modules

        redo main.js
      '';

      installPhase = ''
        install -Dm644 -t $out index.html
        install -Dm644 -t $out openlab-logo.png
        install -Dm644 -t $out main.js
        install -Dm644 -t $out style.css
      '';

      dontStrip = true;
    };

  pythonShell =
    let
      pythonEnv = python3.withPackages (p: with p; [
        flask
        pillow
        requests
        sqlite
      ]);
    in mkShell {
      name = "python-env";

      buildInputs = [ pythonEnv sqlite ];
    };
}

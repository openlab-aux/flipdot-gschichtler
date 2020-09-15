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
      '';

      nativeBuildInputs = [ redo ];

      buildInputs = [ scrypt ];
    };
in

rec {
  warteraum-static = (pkgsMusl.callPackage warteraumDrv {
    stdenv = pkgsMusl.clangStdenv;
    redo = redo-sh;
  }).overrideAttrs (old: {
    # musl, static linking
    patchPhase = ''
      substituteInPlace ./build_config --replace gnu99 c99
      cat >> ./build_config << EOF
      CFLAGS="\$CFLAGS -static"
      EOF
    '';
  });

  warteraum = callPackage warteraumDrv {
    stdenv = clangStdenv;
    redo = redo-sh;
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

{ pkgs ? (import <nixpkgs> {}) }:

with pkgs;

let
  version = "unstable";
  gi = nix-gitignore;
  root = ./.;
  sourceName = "flipdot-gschichtler-source";
in

rec {
  src = builtins.path {
    path = root;
    name = sourceName;
    filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) root;
  };

  warteraum =
    clangStdenvNoLibs.mkDerivation rec {
      pname = "warteraum";

      inherit src version;

      sourceRoot = sourceName + "/warteraum";

      nativeBuildInputs = [ redo-sh ];
      buildInputs = [ musl ];

      # musl, static linking
      patchPhase = ''
        substituteInPlace ./build_config \
          --replace clang musl-clang \
          --replace gnu99 c99
        cat >> ./build_config << EOF
        CFLAGS="\$CFLAGS -static"
        EOF
      '';

      buildPhase = "redo";

      doCheck = true;
      checkPhase = "./test/test_queue";

      installPhase = ''
        install -Dm755 warteraum -t $out/bin
      '';
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

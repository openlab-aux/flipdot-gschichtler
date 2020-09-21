{ pkgs ? (import ./nixpkgs-pinned.nix { })
, scryptSalt ? null
, apiTokens ? null
}:

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

  stringSegments = n: s:
    let
      stringSplitter = i:
        builtins.substring (i * 2) n s;
      nonempty = s: builtins.stringLength s != 0;
    in
      builtins.filter nonempty (builtins.genList
        stringSplitter ((builtins.stringLength s / n) + 1));

  warteraumDrv = { stdenv, redo, scrypt, scryptSalt ? null, apiTokens ? null }:
    let
      saltBytes = stringSegments 2 scryptSalt;
      saltArray =
        let
          commas = builtins.foldl' (a: b: a + ", 0x" + b) "" saltBytes;
        in builtins.substring 1 (builtins.stringLength commas) commas;
      saltReplace = lib.optionalString (scryptSalt != null) ''
        sed -i '/^  0x/d' scrypt.h
        sed -i '/const uint8_t salt/a\${saltArray}' scrypt.h
      '';
      tokensReplace = lib.optionalString (apiTokens != null) ''
        redo hashtoken
        sed -i '/^  {/d' tokens.h
        sed -i '/^};/d' tokens.h
        ${lib.concatMapStringsSep "\n"
            (x: "./hashtoken ${x} >> tokens.h; echo -n ', ' >> tokens.h") apiTokens}
        echo "};" >> tokens.h
      '';
    in stdenv.mkDerivation rec {
      pname = "warteraum";
      sourceRoot = sourceName + "/warteraum";

      inherit src version;

      # make whole source tree writeable for redo
      postUnpack = ''
        chmod -R u+w "$sourceRoot/.."
      '';

      patchPhase = ''
        runHook prePatch

        patchShebangs test/run
        ${saltReplace}
        ${tokensReplace}

        runHook postPatch
      '';

      buildPhase = "redo";

      doCheck = true;
      checkPhase = "./test/run";

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
    redo = pkgsStatic.redo-c;
    inherit scryptSalt apiTokens;
  }).overrideAttrs (old: {
    # musl, static linking
    postPatch = ''
      cat >> ./build_config << EOF
      CFLAGS="\$CFLAGS -static"
      EOF
    '';
  });

  warteraum = callPackage warteraumDrv {
    stdenv = clangStdenv;
    redo = redo-c;
    inherit scryptSalt apiTokens;
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

  overlay = callPackage ./nixos/overlay.nix {
    inherit bahnhofshalle;
    warteraum = warteraum-static;
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

  inherit stringSegments;
}

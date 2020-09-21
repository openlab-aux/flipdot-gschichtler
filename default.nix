{ pkgs ? (import ./nixpkgs-pinned.nix { })
, scryptSalt ? null
, apiTokens ? null
}:

with pkgs;

let
  version = "2.0.0";
  gi = nix-gitignore;
  root = ./.;
  sourceName = "flipdot-gschichtler-source";

  src = builtins.path {
    path = root;
    name = sourceName;
    filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) root;
  };

  stringSegments = n: s:
    let
      stringSplitter = i:
        builtins.substring (i * 2) n s;
      nonempty = s: builtins.stringLength s != 0;
    in
      builtins.filter nonempty (builtins.genList
        stringSplitter ((builtins.stringLength s / n) + 1));

  warteraumDrv = { stdenv, redo, scrypt, jq, scryptSalt ? null, apiTokens ? null }:
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
      checkInputs = [ jq ];

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
      nodePackages = import ./bahnhofshalle/node2nix { inherit pkgs nodejs; };
      nodeDeps = nodePackages.shell.nodeDependencies;
    in stdenv.mkDerivation {
      pname = "bahnhofshalle";
      inherit version;

      inherit src;
      sourceRoot = sourceName + "/bahnhofshalle";

      buildInputs = [ nodejs ];

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
      pythonEnv = python3.withPackages (p: with p; [ pillow requests ]);
      libPath = "$out/lib/${pythonEnv.libPrefix}/site-lib";
    in stdenv.mkDerivation {
      pname = "anzeigetafel";
      inherit version;

      inherit src;
      sourceRoot = sourceName + "/anzeigetafel";

      buildInputs = [ pythonEnv makeWrapper ];

      patchPhase = ''
        sed -i 's|FONT =.*$|FONT = "${unifont}/share/fonts/truetype/unifont.ttf"|' anzeigetafel.py
      '';

      installPhase = ''
        install -Dm755 anzeigetafel.py $out/bin/anzeigetafel
        mkdir -p ${libPath}/flipdots
        cp -r ../third_party/flipdots/scripts ${libPath}/flipdots/scripts
      '';

      postFixup = ''
        wrapProgram $out/bin/anzeigetafel \
          --prefix PYTHONPATH : "${libPath}"
      '';
    };
}

{ stdenv, lib, redo, scrypt, jq
, scryptSalt ? null, apiTokens ? null
, rootSrc, sourceName
}:

let
  stringSegments = n: s:
    let
      stringSplitter = i:
        builtins.substring (i * 2) n s;
      nonempty = s: builtins.stringLength s != 0;
    in
      builtins.filter nonempty (builtins.genList
        stringSplitter ((builtins.stringLength s / n) + 1));

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

in

stdenv.mkDerivation rec {
  pname = "warteraum";
  version = import ./version.nix;

  sourceRoot = sourceName + "/warteraum";

  src = rootSrc;

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
}

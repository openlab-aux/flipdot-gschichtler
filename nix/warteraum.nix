{ stdenv, lib, scrypt
, jq, requests, pytest, pytest-randomly, flipdot-gschichtler, valgrind
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

  makeFlags = [
    "PREFIX=${placeholder "out"}"
  ];

  #postUnpack = ''
  #  chmod -R u+w "$sourceRoot/.."
  #'';

  patchPhase = ''
    runHook prePatch

    patchShebangs test/integration
    ${saltReplace}
    ${tokensReplace}

    runHook postPatch
  '';

  doCheck = true;
  checkInputs = [
    jq valgrind pytest pytest-randomly requests flipdot-gschichtler
  ];

  buildInputs = [ scrypt ];
}

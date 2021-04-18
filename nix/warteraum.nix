{ stdenv, lib, scrypt
, jq, requests, pytest, pytest-randomly, flipdot-gschichtler, valgrind
, rootSrc, sourceName
}:

stdenv.mkDerivation rec {
  pname = "warteraum";
  version = import ./version.nix;

  sourceRoot = sourceName + "/warteraum";

  src = rootSrc;

  makeFlags = [
    "PREFIX=${placeholder "out"}"
  ];

  postPatch = ''
    patchShebangs test/integration
  '';

  doCheck = true;
  checkInputs = [
    jq valgrind pytest pytest-randomly requests flipdot-gschichtler
  ];

  buildInputs = [ scrypt ];
}

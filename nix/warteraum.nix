{ stdenv, lib, scrypt
, jq, requests, pytest, pytest-randomly, flipdot-gschichtler, valgrind
, getSrc
}:

stdenv.mkDerivation rec {
  pname = "warteraum";
  version = import ./version.nix;

  src = [
    (getSrc "warteraum")
    (getSrc "third_party")
  ];

  sourceRoot = "warteraum";

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

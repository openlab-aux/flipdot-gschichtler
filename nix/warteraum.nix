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

  doCheck = lib.meta.availableOn stdenv.hostPlatform valgrind;
  checkInputs = [
    jq valgrind pytest pytest-randomly requests flipdot-gschichtler
  ];

  buildInputs = [ scrypt ];
}

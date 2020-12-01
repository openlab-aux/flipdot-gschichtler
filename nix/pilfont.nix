{ stdenv, fetchurl, python }:

let
  pythonEnv = python.withPackages (p: [ p.pillow ]);
in

stdenv.mkDerivation rec {
  pname = "pilfont";
  version = "unstable-2019-03-07";

  src = fetchurl {
    url = "https://raw.githubusercontent.com/python-pillow/pillow-scripts/b24479cf88d2d9b2cb5518971f3949f318f5c40e/Scripts/pilfont.py";
    sha256 = "15my6380scmni5r2bnrkjgqqf3r38r275h91ygmi7a0935987n13";
  };

  dontUnpack = true;

  buildInputs = [ pythonEnv ];

  installPhase = ''
    mkdir -p $out/bin
    install -Dm755 $src $out/bin/pilfont
  '';

  meta = {
    description = "PIL raster font compiler";
    inherit (python.pkgs.pillow.meta) license;
    homepage = "https://github.com/python-pillow/pillow-scripts";
  };
}

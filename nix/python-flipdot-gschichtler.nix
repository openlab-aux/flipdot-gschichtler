{ buildPythonPackage, requests
, getSrc
}:

buildPythonPackage rec {
  pname = "flipdot-gschichtler";
  version = import ./version.nix;

  src = getSrc "clients/py";

  doCheck = false;

  postPatch = ''
    sed -i "s/version = '.*'/version = '${version}'/" setup.py
  '';

  propagatedBuildInputs = [ requests ];

  pythonImportsCheck = [ "flipdot_gschichtler" ];

  meta = {
    description = "Python client for the flipdot-gschichtler API";
  };
}

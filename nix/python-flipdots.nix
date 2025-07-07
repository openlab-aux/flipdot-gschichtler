{ buildPythonPackage, setuptools, pillow, numpy, flask
, getSrc
}:

buildPythonPackage {
  name = "flipdots";
  version = "unstable";

  src = getSrc "third_party/flipdots";

  pyproject = true;
  build-system = [ setuptools ];

  dependencies = [ flask pillow numpy ];

  doCheck = false;

  pythonImportsCheck = [ "flipdots.scripts" ];

  meta = {
    description = "Scripts and python library to interact with the flipdots";
  };
}

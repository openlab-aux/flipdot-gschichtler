{ buildPythonPackage, pillow, numpy, flask
, rootSrc
}:

buildPythonPackage {
  name = "flipdots";
  version = "unstable";

  src = rootSrc + "/third_party/flipdots";

  propagatedBuildInputs = [ flask pillow numpy ];

  doCheck = false;

  pythonImportsCheck = [ "flipdots.scripts" ];

  meta = {
    description = "Scripts and python library to interact with the flipdots";
  };
}

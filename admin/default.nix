with import <nixpkgs> {};

buildPythonPackage {
  name = "flipdot-gschichtler-admin";

  buildInputs = with pkgs.pythonPackages; [ flask requests jinja2 ];

  src = ./.;
}
with import <nixpkgs> {};

buildPythonPackage {
  name = "flipdot-gschichtler-flipper";

  buildInputs = with pkgs; [ pythonPackages.requests pythonPackages.pillow ];

  src = ./.;
}

with import <nixpkgs> {};

buildPythonPackage {
  name = "flipdot-gschichtler";

  buildInputs = with pkgs; [ sqlite pythonPackages.sqlite3 pythonPackages.flask ];

  src = ./.;
}

with import <nixpkgs> {};

buildPythonPackage {
  name = "flipdot-gschichtler-web";

  buildInputs = with pkgs; [ sqlite pythonPackages.sqlite3 pythonPackages.flask ];

  src = ./.;
}

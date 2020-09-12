{ pkgs ? (import <nixpkgs> {}) }:

with pkgs;

let
  version = "unstable";
  gi = nix-gitignore;
  root = ./.;
in

rec {
  src = builtins.path {
    path = root;
    name = "flipdot-gschichtler-source";
    filter = gi.gitignoreFilter (builtins.readFile ./.gitignore) root;
  };

  pythonShell =
    let
      pythonEnv = python3.withPackages (p: with p; [
        flask
        pillow
        requests
        sqlite
      ]);
    in mkShell {
      name = "python-env";

      buildInputs = [ pythonEnv sqlite ];
    };
}

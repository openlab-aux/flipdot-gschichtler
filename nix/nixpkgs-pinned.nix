let
  # nixos unstable 2021-06-10
  commit = "432fc2d9a67f92e05438dff5fdc2b39d33f77997";
  sha256 = "0npj480pxjzl5vjsbijsn275z48jpqpjp0q5xbwpr3fx6m8lcvr6";
in

import (builtins.fetchTarball {
  url = "https://github.com/nixos/nixpkgs/archive/${commit}.tar.gz";
  inherit sha256;
})

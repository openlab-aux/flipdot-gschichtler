let
  # nixos unstable 2022-02-22
  commit = "7f9b6e2babf232412682c09e57ed666d8f84ac2d";
  sha256 = "03nb8sbzgc3c0qdr1jbsn852zi3qp74z4qcy7vrabvvly8rbixp2";
in

import (builtins.fetchTarball {
  url = "https://github.com/nixos/nixpkgs/archive/${commit}.tar.gz";
  inherit sha256;
})

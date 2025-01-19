let
  # nixos unstable 2025-01-16
  commit = "5df43628fdf08d642be8ba5b3625a6c70731c19c";
  sha256 = "05xhbk4yjbv0f760ld6q9z2v0nphakgk78kgd0wnnmzdjqqkbfad";
in

import (builtins.fetchTarball {
  url = "https://github.com/nixos/nixpkgs/archive/${commit}.tar.gz";
  inherit sha256;
})

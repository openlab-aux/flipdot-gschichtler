let
  # nixos unstable 2025-08-07
  commit = "c2ae88e026f9525daf89587f3cbee584b92b6134";
  sha256 = "1fsnvjvg7z2nvs876ig43f8z6cbhhma72cbxczs30ld0cqgy5dks";
in

import (builtins.fetchTarball {
  url = "https://github.com/nixos/nixpkgs/archive/${commit}.tar.gz";
  inherit sha256;
})

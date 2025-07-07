let
  # nixos unstable 2025-07-07
  commit = "1fd8bada0b6117e6c7eb54aad5813023eed37ccb";
  sha256 = "0jki9azscc2ys89g4qjd61jhsgs3l46rcma7w0395nr3h3m0hn97";
in

import (builtins.fetchTarball {
  url = "https://github.com/nixos/nixpkgs/archive/${commit}.tar.gz";
  inherit sha256;
})

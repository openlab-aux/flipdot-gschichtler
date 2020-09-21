import (builtins.fetchTarball {
  # nixpkgs master 2020-09-15 + scrypt with libscrypt-kdf
  # pin should go to fork only temporarily
  url = "https://github.com/sternenseemann/nixpkgs/archive/scrypt-1.3.1.tar.gz";
  sha256 = "1knvly4w76wb5favmqzz2g4iihqcic757ly20lfxr14yr1ihcmhy";
})

conf="$(dirname "$0")/build_config"
source "$conf"
redo-ifchange "$conf"

redo-ifchange "$2.c"

if [[ -e "$2.h" ]]; then
  redo-ifchange "$2.h"
fi

$CC $CFLAGS -o "$3" -c "$2.c"

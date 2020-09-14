conf=../../warteraum/build_config
source "$conf"

redo-ifchange "$conf" "$2.c"

if [[ -e "$2.h" ]]; then
  redo-ifchange "$2.c"
fi

$CC $CFLAGS -o $3 -c "$2.c"

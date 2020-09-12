source ../build_config
redo-ifchange ../build_config
redo-ifchange "$2.c"

$CC $CFLAGS -o "$3" -c "$2.c"

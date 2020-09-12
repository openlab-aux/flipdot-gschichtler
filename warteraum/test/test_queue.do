source ../build_config
redo-ifchange ../build_config
DEPS="../queue.o test_queue.o"
redo-ifchange test_queue.c
redo-ifchange $DEPS

$CC $CFLAGS -o "$3" $DEPS

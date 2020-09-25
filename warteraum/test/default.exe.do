source ../build_config
redo-ifchange ../build_config
redo-ifchange test.h
redo-ifchange "$2.c"

case "$2" in
  test_queue)
    OBJS="../queue.o"
    ;;
  test_form)
    OBJS="../form.o"
    redo-ifchange ../http_string.h
    ;;
  test_routing)
    OBJS="../routing.o"
    redo-ifchange ../http_string.h
    ;;
esac

redo-ifchange $OBJS

$CC $CFLAGS -o "$3" $OBJS "$2.c"

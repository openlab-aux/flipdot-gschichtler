source ./build_config
redo-ifchange ./build_config
OBJS="queue.o main.o"
DEPS="$OBJS ../third_party/httpserver.h/httpserver.h"
redo-ifchange $DEPS

"$CC" $CFLAGS -o "$3" $OBJS

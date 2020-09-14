source ./build_config
redo-ifchange ./build_config
OBJS="../third_party/json_output/json_output.o queue.o routing.o form.o main.o"
DEPS="$OBJS ../third_party/httpserver.h/httpserver.h"
redo-ifchange $DEPS

"$CC" $CFLAGS -o "$3" $OBJS

source ./build_config
redo-ifchange ./build_config
OBJS="emitjson.o queue.o routing.o form.o main.o announcement.o http_string.o"
DEPS="$OBJS ../third_party/httpserver.h/httpserver.h"
redo-ifchange $DEPS

"$CC" $CFLAGS -o "$3" $OBJS -lscrypt-kdf

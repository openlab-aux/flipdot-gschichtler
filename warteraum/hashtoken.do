conf="./build_config"
redo-ifchange "$conf"
source "$conf"

OBJS="hashtoken.o"
redo-ifchange $OBJS

CFLAGS="$CFLAGS -lscrypt-kdf"

$CC $CFLAGS -o "$3" $OBJS

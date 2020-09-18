conf="./build_config"
redo-ifchange "$conf"
source "$conf"

OBJS="hashtoken.o"
redo-ifchange $OBJS

$CC $CFLAGS -o "$3" $OBJS -lscrypt-kdf

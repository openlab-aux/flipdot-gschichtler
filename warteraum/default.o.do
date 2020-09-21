conf="$(dirname "$0")/build_config"
source "$conf"
redo-ifchange "$conf"

redo-ifchange "$2.c"

if [[ -e "$2.h" ]]; then
  redo-ifchange "$2.h"
fi

case "$2" in
  main)
    redo-ifchange queue.h routing.h form.h v1_static.h
    redo-ifchange scrypt.h tokens.h http_string.h
    redo-ifchange ../third_party/json_output/json_output.h
    redo-ifchange ../third_party/httpserver.h/httpserver.h
    ;;
  form)
    redo-ifchange http_string.h
    redo-ifchange ../third_party/httpserver.h/httpserver.h
    ;;
  routing)
    redo-ifchange ../third_party/httpserver.h/httpserver.h
    ;;
  hashtoken)
    redo-ifchange scrypt.h
    ;;
esac

$CC $CFLAGS -o "$3" -c "$2.c"

#include <stdio.h>
#include <string.h>
#include "../emitjson.h"
#include "test.h"

int main(int argc, char **argv) {
  bool output_json = false;

  if(argc == 2 && strcmp(argv[1], "-o") == 0) {
    output_json = true;
  }

  struct ej_context ctx;
  size_t buf_size;
  char *buf = NULL;
  FILE *out = open_memstream(&buf, &buf_size);

  ej_init(&ctx, out);

  ej_object(&ctx);
  EJ_STATIC_BIND(&ctx, "array");
  ej_array(&ctx);
  ej_uint(&ctx, 25500001);
  ej_int(&ctx, -12000);
  ej_int(&ctx, 10000);
  ej_string(&ctx, "foo\tbar\nbaz\\", 12);
  ej_null(&ctx);
  ej_object(&ctx);
  ej_bind(&ctx, "hello", 5);
  ej_string(&ctx, "world", 5);
  ej_bind(&ctx, "foo\\", 4);
  ej_uint(&ctx, 42);
  ej_object_end(&ctx);
  ej_bool(&ctx, true);
  ej_bool(&ctx, false);
  ej_array_end(&ctx);
  EJ_STATIC_BIND(&ctx, "number");
  ej_uint(&ctx, 1312);
  ej_object_end(&ctx);

  fclose(out);

  bool len_correct = buf_size == ctx.written;

  if(output_json) {
    fwrite(buf, 1, ctx.written, stdout);
  } else {
    test_case("Actually written size matches computed size", len_correct);
  }

  free(buf);

  return !test_result;
}

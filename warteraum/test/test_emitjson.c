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

  EJ_STATIC_BIND(&ctx, "bools and stuff");
  ej_array(&ctx);
  ej_bool(&ctx, true);
  ej_bool(&ctx, false);
  ej_null(&ctx);
  ej_array_end(&ctx);

  EJ_STATIC_BIND(&ctx, "strings");
  ej_array(&ctx);
  ej_string(&ctx, "foo\tbar\nbaz", 11);
  ej_string(&ctx, "form\ffeed", 9);
  ej_string(&ctx, "🤭 unicode 😳", 17);
  ej_array_end(&ctx);

  EJ_STATIC_BIND(&ctx, "objects");
  ej_array(&ctx);
  ej_object(&ctx);
  ej_bind(&ctx, "hello", 5);
  ej_string(&ctx, "world", 5);
  ej_bind(&ctx, "foo\r\nbar", 7);
  ej_uint(&ctx, 42);
  ej_object_end(&ctx);
  ej_object(&ctx);
  ej_object_end(&ctx);
  ej_array_end(&ctx);

  EJ_STATIC_BIND(&ctx, "numbers");
  ej_array(&ctx);
  ej_uint(&ctx, 1312);
  ej_uint(&ctx, 25500001);
  ej_int(&ctx, -12000);
  ej_int(&ctx, 10000);
  ej_long(&ctx, -50000);
  ej_ulong(&ctx, 18340983094);
  ej_long_long(&ctx, 129302193092);
  ej_ulong_long(&ctx, 129302193092);

  ej_int8(&ctx, INT8_MAX);
  ej_int8(&ctx, INT8_MIN);
  ej_int16(&ctx, INT16_MAX);
  ej_int16(&ctx, INT16_MIN);
  ej_int32(&ctx, INT32_MAX);
  ej_int32(&ctx, INT32_MIN);
  ej_int64(&ctx, INT64_MAX);
  ej_int64(&ctx, INT64_MIN);

  ej_uint8(&ctx, UINT8_MAX);
  ej_uint8(&ctx, 0);
  ej_uint16(&ctx, UINT16_MAX);
  ej_uint16(&ctx, 0);
  ej_uint32(&ctx, UINT32_MAX);
  ej_uint32(&ctx, 0);
  ej_uint64(&ctx, UINT64_MAX);
  ej_uint64(&ctx, 0);

  ej_array_end(&ctx);
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

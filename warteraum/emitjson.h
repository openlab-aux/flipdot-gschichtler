#ifndef WARTERAUM_EMITJSON_H
#define WARTERAUM_EMITJSON_H

#include <stdbool.h>

struct ej_context {
  FILE *out;
  bool need_comma;
  size_t written;
};

void ej_init(struct ej_context *, FILE *);

void ej_object(struct ej_context *);
void ej_object_end(struct ej_context *);

#define EJ_STATIC_BIND(ctx, b) \
  ej_bind(ctx, b, sizeof(b) - 1)

void ej_bind(struct ej_context *, const char *, size_t);
//void ej_bind0(struct ej_context *, const char *, size_t);

void ej_array(struct ej_context *);
void ej_array_end(struct ej_context *);

void ej_string(struct ej_context *, const char *, size_t);
//void ej_string0(struct ej_context *, const char *);

void ej_null(struct ej_context *);

void ej_bool(struct ej_context *, bool);

void ej_int(struct ej_context *, int);
void ej_uint(struct ej_context *, unsigned int);

#endif

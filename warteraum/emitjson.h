#ifndef WARTERAUM_EMITJSON_H
#define WARTERAUM_EMITJSON_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

void ej_long(struct ej_context *, long int);
void ej_ulong(struct ej_context *, unsigned long int);
void ej_long_long(struct ej_context *, long long int);
void ej_ulong_long(struct ej_context *, unsigned long long int);

void ej_uint8(struct ej_context *, uint8_t);
void ej_uint16(struct ej_context *, uint16_t);
void ej_uint32(struct ej_context *, uint32_t);
void ej_uint64(struct ej_context *, uint64_t);

void ej_int8(struct ej_context *, int8_t);
void ej_int16(struct ej_context *, int16_t);
void ej_int32(struct ej_context *, int32_t);
void ej_int64(struct ej_context *, int64_t);

#endif

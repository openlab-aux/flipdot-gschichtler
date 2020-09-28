#include <stdio.h>
#include "emitjson.h"

#define BUF_SIZE 512

void ej_putc(struct ej_context *ctx, char c) {
  fputc(c, ctx->out);
  ctx->written++;
}

void ej_init(struct ej_context *ctx, FILE *out) {
  ctx->out = out;
  ctx->need_comma = false;
  ctx->written = 0;
}

void ej_object(struct ej_context *ctx) {
  if(ctx->need_comma) {
    ej_putc(ctx, ',');
  }
  ej_putc(ctx, '{');
  ctx->need_comma = false;
}

void ej_object_end(struct ej_context *ctx) {
  ej_putc(ctx, '}');
  ctx->need_comma = true;
}

void ej_bind(struct ej_context *ctx, const char *b, size_t len) {
  ej_string(ctx, b, len);
  ctx->need_comma = false;

  ej_putc(ctx, ':');
}

void ej_array(struct ej_context *ctx) {
  if(ctx->need_comma) {
    ej_putc(ctx, ',');
  }
  ej_putc(ctx, '[');
  ctx->need_comma = false;
}

void ej_array_end(struct ej_context *ctx) {
  ej_putc(ctx, ']');
  ctx->need_comma = true;
}

void ej_string(struct ej_context *ctx, const char *s, size_t len) {
  if(ctx->need_comma) {
    ej_putc(ctx, ',');
  }

  ctx->need_comma = true;

  ej_putc(ctx, '"');

  for(size_t i = 0; i < len; i++) {
    char c;
    bool escape = false;

    switch(s[i]) {
      case '\t':
        escape = true;
        c = 't';
        break;
      case '\f':
        escape = true;
        c = 'f';
        break;
      case '\r':
        escape = true;
        c = 'r';
        break;
      case '\n':
        escape = true;
        c = 'n';
        break;
      case '\\':
      case '"':
        escape = true;
      default:
        c = s[i];
        break;
    }

    if(escape) {
      ej_putc(ctx, '\\');
    }

    ej_putc(ctx, c);
  }

  ej_putc(ctx, '"');
}

void ej_null(struct ej_context *ctx) {
  if(ctx->need_comma) {
    ej_putc(ctx, ',');
  }

  ctx->need_comma = true;

  size_t wsize = fwrite("null", 1, sizeof("null") - 1, ctx->out);
  ctx->written += wsize;
}

void ej_bool(struct ej_context *ctx, bool b) {
  if(ctx->need_comma) {
    ej_putc(ctx, ',');
  }

  ctx->need_comma = true;

  size_t len;
  char *s;

  if(b) {
    s = "true";
    len = sizeof("true") - 1;
  } else {
    s = "false";
    len = sizeof("false") - 1;
  }

  size_t wsize = fwrite(s, 1, len, ctx->out);
  ctx->written += wsize;
}

// generics for C 🤡
#define EJ_INT_FUN(name, type, sign) \
  void name(struct ej_context *ctx, type u) {                  \
    if(ctx->need_comma) {                                      \
      ej_putc(ctx, ',');                                       \
    }                                                          \
                                                               \
    ctx->need_comma = true;                                    \
                                                               \
    char buf[BUF_SIZE];                                        \
    type d;                                                    \
    size_t len = 0;                                            \
    char *start = buf + BUF_SIZE;                              \
    bool add_sign = false;                                     \
                                                               \
    if(sign && u < 0) {                                        \
      add_sign = true;                                         \
    }                                                          \
                                                               \
    do {                                                       \
      start--;                                                 \
      len++;                                                   \
      d = u % 10;                                              \
      if(add_sign) {                                           \
        d = -d;                                                \
      }                                                        \
      u = u / 10;                                              \
      *start = d + 48;                                         \
    } while(u != 0 && len <= BUF_SIZE);                        \
                                                               \
    if(add_sign) {                                             \
      ej_putc(ctx, '-');                                       \
    }                                                          \
                                                               \
    size_t wsize = fwrite(start, sizeof(char), len, ctx->out); \
    ctx->written += wsize;                                     \
  }

EJ_INT_FUN(ej_uint, unsigned int, false)

EJ_INT_FUN(ej_int, int, true)

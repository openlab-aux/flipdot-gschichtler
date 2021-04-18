#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "http_string.h"

#define BASE_BUF_SIZE 1024

void http_string_clear(struct http_string_s *s) {
  s->buf = NULL;
  s->len = 0;
}

void http_string_free(struct http_string_s *s) {
  if(s->buf != NULL)
    free((void *) s->buf);

  http_string_clear(s);
}

struct http_string_s http_string_fread(char *path) {
  struct http_string_s out;
  http_string_clear(&out);

  FILE *f = fopen(path, "r");

  if(f == NULL)
    return out;

  char  *buf = NULL;
  size_t cap = BASE_BUF_SIZE;
  size_t pos = 0;
  size_t read = 0;

  do {
    char *tmp = realloc(buf, cap);

    if(tmp == NULL) {
      if(buf != NULL) {
        fclose(f);
        free(buf);
      }

      errno = ENOMEM;
      return out;
    }

    buf = tmp;

    read = fread(buf + pos, sizeof(char), cap - pos, f);
    pos += read;
  } while(read > 0 && !(feof(f) || ferror(f)));

  if(!ferror(f)) {
    out.len = pos;
    out.buf = buf;
  } else {
    free(buf);
  }

  fclose(f);

  return out;
}

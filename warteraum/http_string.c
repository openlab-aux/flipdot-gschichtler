#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "http_string.h"

unsigned long long http_string_to_uint(struct http_string_s s) {
  char *zero_terminated = malloc(s.len + 1);
  char *end = NULL;

  if(zero_terminated == NULL) {
    errno = ENOMEM;
    return ULONG_MAX;
  }

  memcpy(zero_terminated, s.buf, s.len);
  zero_terminated[s.len] = '\0';

  unsigned long long int l = strtoull(zero_terminated, &end, 10);

  if(*end != '\0') {
    errno = EINVAL;
  }

  free(zero_terminated);

  return l;
}

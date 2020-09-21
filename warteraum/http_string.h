#ifndef WARTERAUM_HTTP_STRING_H
#define WARTERAUM_HTTP_STRING_H

#include <string.h>

#define STATIC_HTTP_STRING(s) \
  { s, sizeof(s) - 1 }

#define HTTP_STRING_EQ(a, b) \
  (a.len == b.len && strncmp(a.buf, b.buf, a.len) == 0)

#define HTTP_STRING_IS(a, s) \
  (a.len == sizeof(s) - 1 && strncmp(a.buf, s, a.len) == 0)

#endif

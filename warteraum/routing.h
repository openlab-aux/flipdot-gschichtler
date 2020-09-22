#include "../third_party/httpserver.h/httpserver.h"
#include "http_string.h"

int split_segments(struct http_string_s, struct http_string_s **);

#define SEGMENT_MATCH(index, str, segs, count)             \
 ((index < count) && HTTP_STRING_IS(segs[index], str))

#define SEGMENT_MATCH_LAST(index, str, segs, count) \
  (index + 1 == count && SEGMENT_MATCH(index, str, segs, count))

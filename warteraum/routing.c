#include "../third_party/httpserver.h/httpserver.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "routing.h"

int split_segments(struct http_string_s path, struct http_string_s **segs) {
  if(segs == NULL || *segs != NULL) {
    return -1;
  }

  if(path.len < 1 || path.buf[0] != '/') {
    return -1;
  }

  const size_t base_size = 10;
  size_t size = 0;

  size_t seg_len = 0;
  bool in_seg = false;

  for(int i = 0; i < path.len; i++) {
    // make sure there is space for one more after this
    if(seg_len >= size) {
      size += base_size;
      struct http_string_s *tmp;

      if(size > SIZE_MAX/sizeof(struct http_string_s)) {
        break;
      }

      tmp = realloc(*segs, sizeof(struct http_string_s) * size);

      if(tmp == NULL) {
        break;
      }

      *segs = tmp;
    }
    if(path.buf[i] == '/') {
      in_seg = false;
    } else {
      if(in_seg) {
        (*segs)[seg_len - 1].len += 1;
      } else {
        seg_len++;
        (*segs)[seg_len - 1].buf = path.buf + i;
        (*segs)[seg_len - 1].len = 1;
        in_seg = true;
      }
    }
  }

  return seg_len;
}

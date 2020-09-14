#include "../third_party/httpserver.h/httpserver.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

bool segment_match(int index, char *string, struct http_string_s *segs, int seg_count) {
  if(index >= seg_count || segs == NULL || string == NULL) {
    return false;
  }

  return (strncmp(string, segs[index].buf, segs[index].len) == 0);
}

bool segment_match_last(int index, char *string, struct http_string_s *segs, int seg_count) {
  return (index + 1 == seg_count && segment_match(index, string, segs, seg_count));
}

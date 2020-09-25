#include <stdbool.h>
#include <stdio.h>

#define TEST_EXIT_ON_FAIL true
#include "test.h"

#include "../http_string.h"
#include "../routing.h"

int main(void) {
  {
    struct http_string_s root = STATIC_HTTP_STRING("/");
    struct http_string_s *segs = NULL;

    test_case("root returns no segments", split_segments(root, &segs) == 0);

    free(segs);
  }

  {
    struct http_string_s strange = STATIC_HTTP_STRING("/foo/ba%20r/baz?hello=world");
    struct http_string_s *segs = NULL;

    int count = split_segments(strange, &segs);

    test_case("correct amount of segments", count == 3);
    test_case("correct segments", HTTP_STRING_IS(segs[0], "foo")
      && HTTP_STRING_IS(segs[1], "ba%20r")
      && HTTP_STRING_IS(segs[2], "baz?hello=world"));

    free(segs);
  }
}

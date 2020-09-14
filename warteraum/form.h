#include "../third_party/httpserver.h/httpserver.h"

#include <stdbool.h>

enum form_token_type {
  FORM_TOKEN_EQUAL_SIGN,
  FORM_TOKEN_AND_SIGN,
  FORM_TOKEN_STRING
};

struct form_token {
  enum form_token_type type;
  struct http_string_s token;
};

struct form_token_spec {
  enum form_token_type expected;
  struct http_string_s *target;
};

struct form_token *form_next_token(struct http_string_s, int *);

bool form_parse(struct http_string_s, const struct form_token_spec[], size_t);

int urldecode(struct http_string_s, char *, size_t);

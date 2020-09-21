#include "../third_party/httpserver.h/httpserver.h"

#include <stdbool.h>

/* Simple parser for application/x-www-form-urlencoded
 * See: https://url.spec.whatwg.org/#urlencoded-parsing
 * May not conform 100%: “The application/x-www-form-urlencoded
 * format is in many ways an aberrant monstrosity”
 */

enum field_type {
  FIELD_TYPE_STRING,
  FIELD_TYPE_OPTIONAL_STRING
};

struct form_field_spec {
  struct http_string_s field;
  enum field_type type;
  struct http_string_s *target;
};

bool form_parse(struct http_string_s, const struct form_field_spec[], size_t);

#define STATIC_FORM_PARSE(s, sp) \
  form_parse(s, sp, sizeof(sp) / sizeof(struct form_field_spec))

int urldecode(struct http_string_s, char *, size_t);

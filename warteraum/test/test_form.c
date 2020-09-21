#define TEST_EXIT_ON_FAIL true
#include "test.h"
#include "../http_string.h"
#include "../form.h"

int main(void) {
  test_result = true;

  struct http_string_s normal;
  const struct form_field_spec single[] = {
    { STATIC_HTTP_STRING("test"), FIELD_TYPE_STRING, &normal }
  };

  struct http_string_s form_single = STATIC_HTTP_STRING("test=hello");
  test_case("single field parse", STATIC_FORM_PARSE(form_single, single));
  test_case("result matches", HTTP_STRING_IS(normal, "hello"));

  struct http_string_s form_single_garbage =
    STATIC_HTTP_STRING("foo&test=hello+world&one=nope&two=no&bar");
  test_case("parse single field with garbage",
    STATIC_FORM_PARSE(form_single_garbage, single));
  test_case("result matches", HTTP_STRING_IS(normal, "hello+world"));

  struct http_string_s empty_string_end = STATIC_HTTP_STRING("foo=bar&test=");
  struct http_string_s empty_string_mid = STATIC_HTTP_STRING("test=&foo=bar");
  test_case("empty string at end of input parses",
    STATIC_FORM_PARSE(empty_string_end, single) && normal.len == 0);
  test_case("empty string in mid of input parses",
    STATIC_FORM_PARSE(empty_string_mid, single) && normal.len == 0);

  struct http_string_s normal2, normal3;
  const struct form_field_spec multiple[] = {
    { STATIC_HTTP_STRING("one"), FIELD_TYPE_STRING, &normal },
    { STATIC_HTTP_STRING("two"), FIELD_TYPE_STRING, &normal2 },
    { STATIC_HTTP_STRING("three"), FIELD_TYPE_STRING, &normal3 }
  };

  struct http_string_s multiple_clean =
    STATIC_HTTP_STRING("two=2&one=1&three=3");
  struct http_string_s multiple_garbage =
    STATIC_HTTP_STRING("bar=foo&one=1&yak&xyz&two=2&three=3&hello=world");

  test_case("parse fails on missing field",
    !STATIC_FORM_PARSE(form_single_garbage, multiple));
  test_case("multiple field parse",
    STATIC_FORM_PARSE(multiple_clean, multiple));
  test_case("results match", HTTP_STRING_IS(normal, "1")
    && HTTP_STRING_IS(normal2, "2") && HTTP_STRING_IS(normal3, "3"));
  test_case("multiple field parse with garbage",
    STATIC_FORM_PARSE(multiple_garbage, multiple));
  test_case("results match", HTTP_STRING_IS(normal, "1")
    && HTTP_STRING_IS(normal2, "2") && HTTP_STRING_IS(normal3, "3"));

  struct http_string_s required, optional;
  const struct form_field_spec optionals[] = {
    { STATIC_HTTP_STRING("required"), FIELD_TYPE_STRING, &required },
    { STATIC_HTTP_STRING("optional"), FIELD_TYPE_OPTIONAL_STRING, &optional }
  };

  struct http_string_s both =
    STATIC_HTTP_STRING("required=lol&optional=lel");
  struct http_string_s one_missing =
    STATIC_HTTP_STRING("required=lol");
  struct http_string_s both_flag =
    STATIC_HTTP_STRING("optional&required=lol");

  test_case("parse of all values", STATIC_FORM_PARSE(both, optionals));
  test_case("results match", HTTP_STRING_IS(required, "lol")
    && HTTP_STRING_IS(optional, "lel"));
  test_case("parse without one optional value",
    STATIC_FORM_PARSE(one_missing, optionals));
  test_case("missing value is marked correctly", optional.len == -1);
  test_case("parse with one flag-ish value",
    STATIC_FORM_PARSE(both_flag, optionals));
  test_case("present flag is empty string", optional.len == 0);

  return !test_result;
}

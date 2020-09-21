#include <stddef.h>
#include <stdbool.h>
#include "form.h"
#include "http_string.h"

enum form_token_type {
  FORM_TOKEN_EQUAL_SIGN,
  FORM_TOKEN_AND_SIGN,
  FORM_TOKEN_STRING
};

struct form_token {
  enum form_token_type type;
  struct http_string_s token;
};

struct form_token *form_next_token(struct http_string_s s, int *pos) {
  static struct form_token t;

  if(*pos >= s.len) {
    return NULL;
  }

  const char *c = s.buf + *pos;
  t.token.buf = c;
  *pos += 1;

  switch(*c) {
    case '&':
      t.type = FORM_TOKEN_AND_SIGN;
      t.token.len = 1;
      break;
    case '=':
      t.type = FORM_TOKEN_EQUAL_SIGN;
      t.token.len = 1;
      break;
    default:
      t.type = FORM_TOKEN_STRING;
      t.token.len = 1;

      for(; *pos < s.len; (*pos)++) {
        if(s.buf[*pos] == '&' || s.buf[*pos] == '=') {
          break;
        } else {
          t.token.len += 1;
        }
      }
  }

  return &t;
}

enum parser_state {
  FORM_PARSER_IN_KEY,
  FORM_PARSER_IN_CON,
  FORM_PARSER_IN_VAL
};

enum parser_need {
  FORM_PARSER_NEED_KEY = 0x1,
  FORM_PARSER_NEED_VAL = 0x2,
  FORM_PARSER_SKIP     = 0x4
};

bool form_parse(struct http_string_s s, const struct form_field_spec specs[], size_t len) {
  const struct http_string_s empty = STATIC_HTTP_STRING("");
  struct form_token *t;
  int pos = 0;
  size_t remaining = len;
  size_t optionals = 0;

  for(size_t i = 0; i < len && remaining > 0; i++) {
    if(specs[i].type == FIELD_TYPE_OPTIONAL_STRING) {
      remaining -= 1;
      optionals += 1;
    }

    if(specs[i].target != NULL) {
      *(specs[i].target) = empty;
      specs[i].target->len = -1;
    }
  }

  enum parser_state state = FORM_PARSER_IN_KEY;
  enum parser_need need = FORM_PARSER_NEED_KEY;
  size_t current_key_index;
  bool parse_error = false;

  while((t = form_next_token(s, &pos)) != NULL && !parse_error
        && (remaining > 0 || optionals > 0)) {
    struct http_string_s key, val;

    switch(state) {
      case FORM_PARSER_IN_KEY:

        if(t->type != FORM_TOKEN_STRING) {
          key = empty;
        } else {
          key = t->token;
        }

        need = FORM_PARSER_SKIP;

        // TODO binary search over alphabetically ordered?
        for(size_t i = 0; i < len; i++) {
          if(HTTP_STRING_EQ(key, specs[i].field)) {
            current_key_index = i;

            if(specs[i].type == FIELD_TYPE_STRING) {
              need = FORM_PARSER_NEED_VAL;
            } else if(specs[i].type == FIELD_TYPE_OPTIONAL_STRING) {
              need = FORM_PARSER_NEED_VAL | FORM_PARSER_NEED_KEY;

              // set to empty string in case no value is coming
              *(specs[i].target) = empty;
            } else {
              parse_error = true;
            }
            break;
          }
        }

        state = FORM_PARSER_IN_CON;
        break;
      case FORM_PARSER_IN_CON:
        if(t->type == FORM_TOKEN_EQUAL_SIGN) {
          parse_error = !(need & (FORM_PARSER_NEED_VAL | FORM_PARSER_SKIP));
          state = FORM_PARSER_IN_VAL;
        } else if(t->type == FORM_TOKEN_AND_SIGN) {
          parse_error = !(need & (FORM_PARSER_NEED_KEY | FORM_PARSER_SKIP));
          state = FORM_PARSER_IN_KEY;
        } else {
          parse_error = true;
        }
        break;
      case FORM_PARSER_IN_VAL:
        if(t->type != FORM_TOKEN_STRING) {
          val = empty;
        } else {
          val = t->token;
        }

        if(need & FORM_PARSER_NEED_VAL) {
          if(specs[current_key_index].target != NULL) {
            *(specs[current_key_index].target) = val;
            remaining -= remaining > 0 ? 1 : 0;
          }
        }

        need = FORM_PARSER_NEED_KEY;
        state = FORM_PARSER_IN_CON;
    }
  }

  if(state == FORM_PARSER_IN_VAL && (need & FORM_PARSER_NEED_VAL)) {
    *(specs[current_key_index].target) = empty;
    remaining -= remaining > 0 ? 1 : 0;
  }

  return (!parse_error && remaining == 0);
}

char hex_to_int(char c) {
    if(c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

int urldecode(struct http_string_s s, char *target, size_t size) {
  size_t len = 0;
  int pos = 0;

  if(target == NULL || s.len < 0 || s.buf == NULL) {
    return -1;
  }

  while(pos < s.len && (size_t) ++len <= size) {
    char c = s.buf[pos];

    if(c == '%') {
      if(pos + 2 < s.len) {
        int a = hex_to_int(s.buf[++pos]);
        int b = hex_to_int(s.buf[++pos]);
        target[len - 1] = (a << 4) + b;
      } else {
        return -1;
      }
    } else if(c == '+') {
      target[len - 1] = ' ';
    } else {
      target[len - 1] = c;
    }
    pos++;
  }

  return len;
}

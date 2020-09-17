#include <stddef.h>
#include <stdbool.h>
#include "form.h"

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

bool form_parse(struct http_string_s s, const struct form_token_spec specs[], size_t len) {
  struct form_token *t;
  int pos = 0;

  for(size_t i = 0; i < len; i++) {
    t = form_next_token(s, &pos);

    if(t == NULL || t->type != specs[i].expected) {
      return 0;
    }

    if(specs[i].target != NULL) {
      *(specs[i].target) = t->token;
    }
  }

  return 1;
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

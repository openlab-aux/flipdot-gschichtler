#define HTTPSERVER_IMPL
#include "../third_party/httpserver.h/httpserver.h"

#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "emitjson.h"

#include "http_string.h"

#include "announcement.h"
#include "queue.h"
#include "routing.h"
#include "form.h"
#include "scrypt.h"

#include "v1_static.h" /* static strings for v1 api */
#include "tokens.h"    /* valid api tokens */

#define LISTEN_PORT    9000   /* port to listen on          */
#define MAX_BODY_LEN   8192   /* max body size we'll parse  */
#define MAX_TEXT_LEN   512    /* max length of a queue text */

// compare http_string against a static string,
// but optionally allow an ;… after it.
// i.e. application/json;charset=utf8 matches with
// application/json
#define MATCH_CONTENT_TYPE(a, s)                             \
  ((size_t) a.len >= sizeof(s) - 1 &&                        \
    strncmp(a.buf, s, sizeof(s) - 1) == 0 &&                 \
    ((size_t) a.len < sizeof(s) || a.buf[sizeof(s) - 1] == ';'))

#define INTERNAL_ERROR_STATIC \
  "{\"error\":\"internal error while building error response\"}"

// Global state

static struct queue flip_queue;
static struct http_server_s* server;

static struct warteraum_announcement announcement;

void cleanup(int signum) {
  if(signum == SIGTERM || signum == SIGINT) {
    queue_free(flip_queue);
    free(server);
    announcement_delete(&announcement);
    exit(EXIT_SUCCESS);
  }
}

// adjust pointer and length so it points to a string that
// has no leading nor trailing whitespace
void trim_whitespace(struct http_string_s *s) {
  const char *str = s->buf;
  int len = s->len;

  if(len > 0) {
    int new_len = len;
    int new_start = 0;

    int pos = 0;
    while(isspace(*(str + pos)) && pos < len) {
      new_start++;
      new_len--;
      pos++;
    }

    if(new_len > 0) {
      pos = len - 1;
      while(isspace(*(str + pos)) && pos > new_start) {
        new_len--;
        pos--;
      }
    }

    s->len = new_len;
    s->buf = str + new_start;
  }
}

// authentication

bool authenticate(http_string_t token) {
  uint8_t hashed[SCRYPT_OUTPUT_LEN];

  int hash_result = HASH_TOKEN(token.buf, token.len, hashed);

  if(hash_result != 0) {
    return false;
  }

  bool token_matches = false;
  size_t token_count = sizeof(tokens) / (sizeof(uint8_t) * SCRYPT_OUTPUT_LEN);

  for(size_t i = 0; i < token_count && !token_matches; i++) {
    token_matches = true;
    for(size_t j = 0; j < SCRYPT_OUTPUT_LEN && token_matches; j++) {
      token_matches = tokens[i][j] == hashed[j];
    }
  }

  return token_matches;
}

// main routing logic

enum warteraum_result {
  WARTERAUM_OK = 0,
  WARTERAUM_BAD_REQUEST = 1,
  WARTERAUM_UNAUTHORIZED = 2,
  WARTERAUM_NOT_FOUND = 3,
  WARTERAUM_INTERNAL_ERROR = 4,
  WARTERAUM_FULL_ERROR = 5,
  WARTERAUM_ENTRY_NOT_FOUND = 6,
  WARTERAUM_TOO_LONG = 7
};

enum warteraum_version {
  WARTERAUM_API_V1,
  WARTERAUM_API_V2
};

void response_error(enum warteraum_result e, bool legacy_response, http_request_t *request, http_response_t *response) {
  // response_error should never be called with
  // WARTERAUM_OK, so this is considered a error 500
  const http_string_t errors[] = {
    STATIC_HTTP_STRING("internal server error"),
    STATIC_HTTP_STRING("bad request"),
    STATIC_HTTP_STRING("unauthorized"),
    STATIC_HTTP_STRING("endpoint not found"),
    STATIC_HTTP_STRING("internal server error"),
    STATIC_HTTP_STRING("queue is full (max id reached)"),
    STATIC_HTTP_STRING("queue entry not found"),
    STATIC_HTTP_STRING("body or other input too long"),
  };

  const int codes[] = { 500, 400, 401, 404, 500, 503, 404, 413 };

  if(legacy_response) {
    // /api/v1/queue/add returns a HTML response
    // we emulate this behavior, however not exactly

    http_response_status(response, codes[e]);
    http_response_header(response, "Content-Type", "text/html");
    http_response_body(response, QUEUE_ADD_V1_FAILURE, sizeof(QUEUE_ADD_V1_FAILURE) - 1);
    http_respond(request, response);
  } else {
    size_t buf_size = 0;
    char *buf = NULL;
    FILE *out = open_memstream(&buf, &buf_size);
    struct ej_context ctx;
    bool static_buf = false;

    ej_init(&ctx, out);

    if(out == NULL) {
      buf = INTERNAL_ERROR_STATIC;
      buf_size = sizeof(INTERNAL_ERROR_STATIC) - 1;

      static_buf = true;
      e = WARTERAUM_INTERNAL_ERROR;
    } else {
      ej_object(&ctx);
      EJ_STATIC_BIND(&ctx, "error");
      ej_string(&ctx, errors[e].buf, (size_t) errors[e].len);
      ej_object_end(&ctx);

      fclose(out);
    }

    http_response_status(response, codes[e]);
    http_response_header(response, "Content-Type", "application/json");
    http_response_body(response, buf, static_buf ? buf_size : (int) ctx.written);
    http_respond(request, response);

    if(!static_buf) {
      free(buf);
    }
  }
}

// GET /api/{v1, v2}/queue
enum warteraum_result response_queue(enum warteraum_version v, http_request_t *request, http_response_t *response) {
  (void) v; // surpress warning for now

  unsigned int queue_length = 0;

  struct ej_context ctx;
  size_t buf_size = 0;
  char *buf = NULL;
  FILE *out = open_memstream(&buf, &buf_size);

  if(out == NULL) {
    return WARTERAUM_INTERNAL_ERROR;
  }

  ej_init(&ctx, out);

  ej_object(&ctx);
  EJ_STATIC_BIND(&ctx, "queue");
  ej_array(&ctx);

  queue_foreach(flip_queue, elem) {
    queue_length++;
    ej_object(&ctx);

    EJ_STATIC_BIND(&ctx, "id");
    ej_uint(&ctx, elem->id);

    EJ_STATIC_BIND(&ctx, "text");
    ej_string(&ctx, elem->string, elem->string_size);

    ej_object_end(&ctx);
  }

  ej_array_end(&ctx);

  EJ_STATIC_BIND(&ctx, "length");
  ej_uint(&ctx, queue_length);
  ej_object_end(&ctx);

  fclose(out);

  http_response_status(response, 200);
  http_response_header(response, "Content-Type", "application/json");
  http_response_body(response, buf, (int) ctx.written);
  http_respond(request, response);

  free(buf);

  return WARTERAUM_OK;
}

// POST /api/{v1,v2}/queue/add
enum warteraum_result response_queue_add(enum warteraum_version version, http_request_t *request, http_response_t *response) {
  http_string_t text;
  const struct form_field_spec request_spec[] = {
    { STATIC_HTTP_STRING("text"), FIELD_TYPE_STRING, &text }
  };

  if(flip_queue.last != NULL && flip_queue.last->id == QUEUE_MAX_ID) {
    return WARTERAUM_FULL_ERROR;
  }

  http_string_t content_type = http_request_header(request, "Content-Type");
  http_string_t method = http_request_method(request);

  if(!MATCH_CONTENT_TYPE(content_type, "application/x-www-form-urlencoded") ||
     !HTTP_STRING_IS(method, "POST")) {
    return WARTERAUM_BAD_REQUEST;
  }

  http_string_t body = http_request_body(request);

  if(body.len > MAX_BODY_LEN) {
    return WARTERAUM_TOO_LONG;
  }

  bool parse_res = STATIC_FORM_PARSE(body, request_spec);

  if(!parse_res) {
    return WARTERAUM_BAD_REQUEST;
  }

  char *decoded_mem = malloc(text.len); // so we can advance the decoded pointer
  http_string_t decoded;

  if(decoded_mem == NULL) {
    return WARTERAUM_INTERNAL_ERROR;
  }

  decoded.len = urldecode(text, decoded_mem, sizeof(char) * text.len);
  decoded.buf = decoded_mem;

  trim_whitespace(&decoded);

  if(decoded.len <= 0) {
    free(decoded_mem);
    return WARTERAUM_BAD_REQUEST;
  }

  if(decoded.len > MAX_TEXT_LEN) {
    free(decoded_mem);
    return WARTERAUM_TOO_LONG;
  }

  queue_append(&flip_queue, decoded.buf, (size_t) decoded.len);

  free(decoded_mem);

  if(flip_queue.last == NULL) {
    return WARTERAUM_INTERNAL_ERROR;
  }

  if(version == WARTERAUM_API_V1) {
    http_response_status(response, 200);
    http_response_header(response, "Content-Type", "text/html");
    http_response_body(response, QUEUE_ADD_V1_SUCCESS, sizeof(QUEUE_ADD_V1_SUCCESS) - 1);
    http_respond(request, response);
  } else {
    struct ej_context ctx;
    char *buf = NULL;
    size_t buf_size = 0;
    FILE *out = open_memstream(&buf, &buf_size);

    if(out == NULL) {
      return WARTERAUM_INTERNAL_ERROR;
    }

    ej_init(&ctx, out);

    ej_object(&ctx);
    EJ_STATIC_BIND(&ctx, "id");
    ej_uint(&ctx, flip_queue.last->id);
    EJ_STATIC_BIND(&ctx, "text");
    ej_string(&ctx, flip_queue.last->string, flip_queue.last->string_size);
    ej_object_end(&ctx);

    fclose(out);

    http_response_status(response, 200);
    http_response_header(response, "Content-Type", "application/json");
    http_response_body(response, buf, (int) ctx.written);
    http_respond(request, response);

    free(buf);
  }

  return WARTERAUM_OK;
}

// DELETE /api/v1/queue/del/<id>
// DELTE /api/v2/queue/<id>
enum warteraum_result response_queue_del(http_string_t id_str, enum warteraum_version v, http_request_t *request, http_response_t *response) {
  (void) v; // surpress warning for now

  http_string_t content_type = http_request_header(request, "Content-Type");
  http_string_t method = http_request_method(request);

  if(!MATCH_CONTENT_TYPE(content_type, "application/x-www-form-urlencoded") ||
     !HTTP_STRING_IS(method, "DELETE")) {
    return WARTERAUM_BAD_REQUEST;
  }

  http_string_t body = http_request_body(request);
  http_string_t token;

  if(body.len > MAX_BODY_LEN) {
    return WARTERAUM_TOO_LONG;
  }

  const struct form_field_spec request_spec[] = {
    { STATIC_HTTP_STRING("token"), FIELD_TYPE_STRING, &token }
  };

  bool parse_res = STATIC_FORM_PARSE(body, request_spec);

  if(!parse_res) {
    return WARTERAUM_BAD_REQUEST;
  }

  errno = 0;
  bool token_matches = authenticate(token);

  if(errno != 0) {
    // scrypt failed
    return WARTERAUM_INTERNAL_ERROR;
  }

  if(!token_matches) {
    return WARTERAUM_UNAUTHORIZED;
  }

  errno = 0;
  unsigned long int id = http_string_to_uint(id_str);

  // check for conversion errors
  // also abort if id is greater than max id
  if(errno != 0 || id > QUEUE_MAX_ID) {
    return WARTERAUM_BAD_REQUEST;
  }

  if(flip_queue.first == NULL || flip_queue.last == NULL) {
    return WARTERAUM_ENTRY_NOT_FOUND;
  }

  // don't iterate through the queue if the id is out of range
  if(flip_queue.first->id > id || flip_queue.last->id < id) {
    return WARTERAUM_ENTRY_NOT_FOUND;
  }

  bool found = queue_remove(&flip_queue, id);

  if(found) {
    http_response_status(response, 204);
    http_respond(request, response);
    return WARTERAUM_OK;
  } else {
    return WARTERAUM_ENTRY_NOT_FOUND;
  }
}

int make_announcement_response(struct ej_context *ctx) {
  int status;

  ej_object(ctx);
  EJ_STATIC_BIND(ctx, "announcement");

  if(announcement.text.len > 0 && announcement.text.buf != NULL) {
    ej_string(ctx, announcement.text.buf, announcement.text.len);
    status = 200;
  } else {
    ej_null(ctx);
    status = 404;
  }

  EJ_STATIC_BIND(ctx, "expiry_utc");
  if(announcement.announcement_expires) {
    ej_int(ctx, announcement.announcement_expiry);
  } else {
    ej_null(ctx);
  }

  ej_object_end(ctx);

  return status;
}

// GET, PUT /api/v2/announcement
enum warteraum_result response_announcement(enum warteraum_version v, http_request_t *request, http_response_t *response) {
  (void) v; // surpress warnings

  // instead of using a separate thread or something
  // we check expiry every time it is requested
  if(announcement_expired(announcement)) {
    announcement_delete(&announcement);
  }

  http_string_t method = http_request_method(request);

  if(HTTP_STRING_IS(method, "GET") || HTTP_STRING_IS(method, "PUT")) {
    int status = 200;

    struct ej_context ctx;
    size_t buf_size = 0;
    char *buf = NULL;
    FILE *out = open_memstream(&buf, &buf_size);

    if(out == NULL) {
      return WARTERAUM_INTERNAL_ERROR;
    }

    ej_init(&ctx, out);

    if(HTTP_STRING_IS(method, "GET")) {
      status = make_announcement_response(&ctx);
    } else if(HTTP_STRING_IS(method, "PUT")) {
      http_string_t content_type = http_request_header(request, "Content-Type");

      if(!MATCH_CONTENT_TYPE(content_type, "application/x-www-form-urlencoded")) {
        fclose(out);
        free(buf);
        return WARTERAUM_BAD_REQUEST;
      }

      http_string_t body = http_request_body(request);

      if(body.len > MAX_BODY_LEN) {
        fclose(out);
        free(buf);
        return WARTERAUM_TOO_LONG;
      }

      http_string_t text;
      http_string_t token;
      http_string_t expiry_utc_str;

      const struct form_field_spec text_body_spec[] = {
        { STATIC_HTTP_STRING("text"), FIELD_TYPE_STRING, &text },
        { STATIC_HTTP_STRING("token"), FIELD_TYPE_STRING, &token },
        { STATIC_HTTP_STRING("expiry_utc"), FIELD_TYPE_OPTIONAL_STRING, &expiry_utc_str },
      };

      bool parse_result = STATIC_FORM_PARSE(body, text_body_spec);

      if(!parse_result) {
        fclose(out);
        free(buf);
        return WARTERAUM_BAD_REQUEST;
      }

      errno = 0;
      bool token_matches = authenticate(token);

      if(errno != 0) {
        fclose(out);
        free(buf);
        return WARTERAUM_INTERNAL_ERROR;
      }

      if(!token_matches) {
        fclose(out);
        free(buf);
        return WARTERAUM_UNAUTHORIZED;
      }

      http_string_t decoded;
      char *decoded_mem = malloc(text.len);

      if(decoded_mem == NULL) {
        fclose(out);
        free(buf);
        return WARTERAUM_INTERNAL_ERROR;
      }

      decoded.len = urldecode(text, decoded_mem, (size_t) text.len);
      decoded.buf = decoded_mem;

      trim_whitespace(&decoded);

      if(decoded.len <= 0) {
        free(decoded_mem);
        fclose(out);
        free(buf);
        return WARTERAUM_BAD_REQUEST;
      }

      bool update_result = false;

      // check if we have an expiry time
      if(expiry_utc_str.len == -1) {
        update_result = announcement_set(&announcement, decoded) &&
          (make_announcement_response(&ctx) == 200);
      } else {
        time_t expiry_utc;

        http_string_t expiry_utc_decoded;
        char *expiry_utc_decoded_mem = malloc(expiry_utc_str.len);

        if(expiry_utc_decoded_mem == NULL) {
          fclose(out);
          free(buf);
          free(decoded_mem);
          return WARTERAUM_INTERNAL_ERROR;
        }

        expiry_utc_decoded.len = urldecode(expiry_utc_str, expiry_utc_decoded_mem, (size_t) expiry_utc_str.len);
        expiry_utc_decoded.buf = expiry_utc_decoded_mem;

        if(expiry_utc_decoded.len <= 0) {
          free(decoded_mem);
          free(expiry_utc_decoded_mem);
          fclose(out);
          free(buf);
          return WARTERAUM_BAD_REQUEST;
        }

        bool valid = true;

        // check if its a proper number
        for(int i = 0; i < expiry_utc_decoded.len; i++) {
          if(!isdigit(expiry_utc_decoded.buf[i])) {
            valid = false;
          }
        }

        if(valid) {
          errno = 0;
          unsigned long long tmp = http_string_to_uint(expiry_utc_decoded);

          if(errno != 0 || tmp > LONG_MAX) {
            valid = false;
          } else {
            expiry_utc = (time_t) tmp;
          }
        }

        if(!valid) {
          free(decoded_mem);
          free(expiry_utc_decoded_mem);
          fclose(out);
          free(buf);
          return WARTERAUM_BAD_REQUEST;
        }

        update_result = announcement_set_expiring(&announcement, decoded, expiry_utc) &&
          (make_announcement_response(&ctx) == 200);

        free(expiry_utc_decoded_mem);
      }

      free(decoded_mem);

      if(!update_result) {
        fclose(out);
        free(buf);
        return WARTERAUM_INTERNAL_ERROR;
      }
    }

    fclose(out);

    http_response_status(response, status);
    http_response_header(response, "Content-Type", "application/json");
    http_response_body(response, buf, (int) ctx.written);
    http_respond(request, response);

    free(buf);
  } else if(HTTP_STRING_IS(method, "DELETE")) {
    http_string_t content_type = http_request_header(request, "Content-Type");

    if(!MATCH_CONTENT_TYPE(content_type, "application/x-www-form-urlencoded")) {
      return WARTERAUM_BAD_REQUEST;
    }

    http_string_t body = http_request_body(request);

    if(body.len > MAX_BODY_LEN) {
      return WARTERAUM_TOO_LONG;
    }

    http_string_t token;
    const struct form_field_spec token_body_spec[] = {
      { STATIC_HTTP_STRING("token"), FIELD_TYPE_STRING, &token }
    };

    bool parse_result = STATIC_FORM_PARSE(body, token_body_spec);

    if(!parse_result) {
      return WARTERAUM_BAD_REQUEST;
    }

    errno = 0;
    bool token_matches = authenticate(token);
    if(errno != 0) {
      // scrypt failed
      return WARTERAUM_INTERNAL_ERROR;
    }

    if(!token_matches) {
      return WARTERAUM_UNAUTHORIZED;
    }

    announcement_delete(&announcement);

    http_response_status(response, 204);
    http_respond(request, response);
  } else {
    return WARTERAUM_BAD_REQUEST;
  }

  // common for GET and PUT

  // we always return okay, since we want a custom 404 response if
  // we don't have an announcement
  return WARTERAUM_OK;
}

void handle_request(http_request_t *request) {
  // TODO remove this for production?
  // Sending Connection: close avoids memory leaks
  // when terminating the program
  http_request_connection(request, HTTP_CLOSE);

  struct http_response_s* response = http_response_init();

  struct http_string_s target = http_request_target(request);

  struct http_string_s *segs = NULL;
  int count = split_segments(target, &segs);

  enum warteraum_result status = WARTERAUM_NOT_FOUND;
  enum warteraum_version api_version;
  bool v1_html_response = false;

  if(count < 0) {
    status = WARTERAUM_INTERNAL_ERROR;
  } else {
    if(SEGMENT_MATCH(0, "api", segs, count)) {
      if(SEGMENT_MATCH(1, "v1", segs, count)) {
        api_version = WARTERAUM_API_V1;

        if(SEGMENT_MATCH(2, "queue", segs, count)) {
          if(count == 3) {
            status = response_queue(api_version, request, response);
          } else if(SEGMENT_MATCH_LAST(3, "add", segs, count)) {
            // this endpoint returns html in /api/v1
            v1_html_response = true;
            status = response_queue_add(api_version, request, response);
          } else if(SEGMENT_MATCH(3, "del", segs, count) && count == 5) {
            status = response_queue_del(segs[4], api_version, request, response);
          }
        }
      } else if(SEGMENT_MATCH(1, "v2", segs, count)) {
        api_version = WARTERAUM_API_V2;

        if(SEGMENT_MATCH(2, "queue", segs, count)) {
          if(count == 3) {
            status = response_queue(api_version, request, response);
          } else if(SEGMENT_MATCH_LAST(3, "add", segs, count)) {
            status = response_queue_add(api_version, request, response);
          } else if(count == 4) {
            // /api/v2/queue/<id>
            status = response_queue_del(segs[3], api_version, request, response);
          }
        } else if(SEGMENT_MATCH_LAST(2, "announcement", segs, count)) {
          status = response_announcement(api_version, request, response);
        }
      }
    }
  }

  free(segs);

  if(status != WARTERAUM_OK) {
    response_error(status, v1_html_response, request, response);
  }
}

int main(void) {
  queue_new(&flip_queue);
  announcement_new(&announcement);

  signal(SIGTERM, cleanup);
  signal(SIGINT, cleanup);

  server = http_server_init(LISTEN_PORT, handle_request);
  http_server_listen(server);

  return 0;
}

#define HTTPSERVER_IMPL
#include "../third_party/httpserver.h/httpserver.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "emitjson.h"

#include "http_string.h"

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

void cleanup(int signum) {
  if(signum == SIGTERM || signum == SIGINT) {
    queue_free(flip_queue);
    free(server);
    exit(EXIT_SUCCESS);
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

  char *decoded = malloc(sizeof(char) * text.len);
  char *decoded_mem = decoded; // so we can advance the decoded pointer

  if(decoded == NULL) {
    return WARTERAUM_INTERNAL_ERROR;
  }

  int decoded_len = urldecode(text, decoded, sizeof(char) * text.len);

  if(decoded_len <= 0) {
    free(decoded_mem);
    return WARTERAUM_BAD_REQUEST;
  }

  if(decoded_len > MAX_TEXT_LEN) {
    free(decoded_mem);
    return WARTERAUM_TOO_LONG;
  }

  queue_append(&flip_queue, decoded, (size_t) decoded_len);

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

  char *id_zero_terminated = malloc(sizeof(char) * (id_str.len + 1));
  if(id_zero_terminated == NULL) {
    return WARTERAUM_INTERNAL_ERROR;
  }

  memcpy(id_zero_terminated, id_str.buf, id_str.len);
  id_zero_terminated[id_str.len] = '\0';

  errno = 0;
  unsigned long int id = strtoul(id_zero_terminated, NULL, 10);

  free(id_zero_terminated);

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

  signal(SIGTERM, cleanup);
  signal(SIGINT, cleanup);

  server = http_server_init(LISTEN_PORT, handle_request);
  http_server_listen(server);

  return 0;
}

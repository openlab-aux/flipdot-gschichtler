#define HTTPSERVER_IMPL
#include "../third_party/httpserver.h/httpserver.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "queue.h"

#define LISTEN_PORT 9000

// Global state

static struct queue *flip_queue;
static struct http_server_s* server;

void cleanup(int signum) {
  if(signum == SIGTERM || signum == SIGINT) {
    queue_free(flip_queue);
    free(server);
    exit(EXIT_SUCCESS);
  } else {
    fputs("Unexpected signal\n", stderr);
  }
}

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

// main routing logic

void handle_request(struct http_request_s* request) {
  // TODO remove this for production?
  // Sending Connection: close avoids memory leaks
  // when terminating the program
  http_request_connection(request, HTTP_CLOSE);

  struct http_response_s* response = http_response_init();

  struct http_string_s target = http_request_target(request);

  struct http_string_s *segs = NULL;
  int count = split_segments(target, &segs);

  if(count < 0) {
    fputs("Split failure\n", stderr);
  } else {
    for(int i = 0; i < count; i++) {
      fwrite(segs[i].buf, sizeof(char), segs[i].len, stderr);
      fputc(' ', stderr);
    }
    fputc('\n', stderr);
  }

  free(segs);

  http_response_status(response, 404);
  http_response_header(response, "Content-Type", "text/plain");
  http_response_body(response, "not found", sizeof("not found") - 1);
  http_respond(request, response);
}

int main(void) {
  flip_queue = NULL;

  signal(SIGTERM, cleanup);
  signal(SIGINT, cleanup);

  server = http_server_init(LISTEN_PORT, handle_request);
  http_server_listen(server);

  return 0;
}

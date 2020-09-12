#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void queue_free(struct queue *queue) {
  if(queue != NULL) {
    struct queue *tail = queue->tail;
    free(queue->string);
    free(queue);
    queue_free(tail);
  }
}

void queue_pop(struct queue **queue) {
  if(queue != NULL && *queue != NULL) {
    struct queue *tail = (*queue)->tail;
    free((*queue)->string);
    free(*queue);
    *queue = tail;
  }
}

void queue_remove(struct queue **queue, unsigned int id) {
  struct queue **p = queue;
  bool search = true;

  while(search) {
    if((*p)->id == id) {
      search = false;
      struct queue *tail = (*p)->tail;

      free((*p)->string);
      free(*p);

      *p = tail;
    } else {
      p = &((*p)->tail);
    }
  }
}

void queue_push(struct queue **queue, char *str, size_t size) {
  struct queue *tail = *queue;
  unsigned int id;

  if(tail == NULL) {
    id = 0;
  } else {
    id = tail->id + 1;
  }

  *queue = malloc(sizeof(struct queue));
  (*queue)->id = id;
  (*queue)->tail = tail;
  (*queue)->string_size = size;
  (*queue)->string = malloc(sizeof(char) * size);
  memcpy((*queue)->string, str, sizeof(char) * size);
}

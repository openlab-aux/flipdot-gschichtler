#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void queue_free_stack(struct queue_stack *queue) {
  if(queue != NULL) {
    struct queue_stack *tail = queue->tail;
    free(queue->string);
    free(queue);
    queue_free_stack(tail);
  }
}

void queue_free(struct queue queue) {
  queue_free_stack(queue.first);
}

void queue_new(struct queue *q) {
  q->first = NULL;
  q->last = NULL;
}

void queue_pop(struct queue *queue) {
  if(queue->first != NULL) {
    queue_remove(queue, queue->first->id);
  }
}

bool queue_remove(struct queue *queue, unsigned int id) {
  struct queue_stack **p = &(queue->first);
  struct queue_stack *second_to_last = NULL;
  bool search = true;

  while(search && *p != NULL) {
    if((*p)->tail == queue->last) {
      second_to_last = *p;
    }

    if((*p)->id == id) {
      search = false;
      struct queue_stack *tail = (*p)->tail;

      free((*p)->string);
      free(*p);

      *p = tail;

      if(tail == NULL) {
        queue->last = second_to_last;
      }
    } else {
      p = &((*p)->tail);
    }
  }

  return !search;
}

void queue_append(struct queue *queue, const char *str, size_t size) {
  struct queue_stack *new = malloc(sizeof(struct queue_stack));

  if(new == NULL) {
    // TODO
    return;
  }

  new->string_size = size;
  new->string = malloc(sizeof(char) * size);
  new->tail = NULL;

  if(new->string == NULL) {
    free(new);
    return;
  }

  memcpy(new->string, str, size);

  if(queue->last == NULL) {
    new->id = 0;
    queue->first = new;
    queue->last = new;
  } else {
    new->id = queue->last->id + 1;
    queue->last->tail = new;
    queue->last = new;
  }
}

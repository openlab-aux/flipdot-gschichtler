struct queue {
  unsigned int id;
  char *string;
  size_t string_size;
  struct queue *tail;
};

char *queue_peek(struct queue *);
void queue_pop(struct queue **);
void queue_push(struct queue **, char *, size_t);
void queue_remove(struct queue **, unsigned int);
void queue_free(struct queue *);

#define queue_foreach(input, head)                              \
  for(struct queue *head = input,                               \
        *next = head != NULL ? head->tail : NULL;  \
      head != NULL;                                             \
      head = next,                                              \
        next = next != NULL ? next->tail : NULL)

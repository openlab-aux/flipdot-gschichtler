struct queue_stack {
  unsigned int id;
  size_t string_size;
  char *string;
  struct queue_stack *tail;
};

struct queue {
  struct queue_stack *first;
  struct queue_stack *last;
};

void queue_new(struct queue *);
void queue_pop(struct queue *);
void queue_append(struct queue *, const char *, size_t);
void queue_remove(struct queue *, unsigned int);
void queue_free(struct queue);

#define queue_foreach(input, head)                              \
  for(struct queue_stack *head = input.first,                   \
        *next = head != NULL ? head->tail : NULL;               \
      head != NULL;                                             \
      head = next,                                              \
        next = next != NULL ? next->tail : NULL)

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../queue.h"

#define INFO_WIDTH 50

static bool test_result;

void test_case(char *info, bool result) {
  char *result_str = "okay";
  FILE *output = stdout;

  if(!result) {
    result_str = "FAIL";
    output = stderr;
  }

  int w = INFO_WIDTH;

  while(*info != '\0') {
    fputc(*info, output);
    w--; info++;
  }

  fputs(":", output);
  w--;

  while(w > 0) {
    fputc(' ', output);
    w--;
  }

  fputs(info, output);
  fputs(result_str, output);
  fputc('\n', output);

  test_result = test_result && result;
}

int main(void) {
  struct queue *q = NULL;
  test_result = 1;

  queue_push(&q, "test", 5);     // id: 0
  queue_push(&q, "hello", 6);    // id: 1
  queue_push(&q, "bizarre", 8);  // id: 2
  queue_push(&q, "world", 6);    // id: 3
  queue_push(&q, "!", 2);        // id: 4

  test_case("id of queue head", q->id == 4);

  queue_pop(&q);
  test_case("id after pop", q->id == 3);
  test_case("content after pop", strcmp(q->string, "world") == 0);

  queue_remove(&q, 2);
  test_case("id of queue head intact after remove", q->id == 3);

  queue_pop(&q);
  test_case("id after pop", q->id == 1);
  test_case("content after pop", strcmp(q->string, "hello") == 0);

  queue_push(&q, "stranger", 9);

  unsigned int count = 0;
  queue_foreach(q, elem) {
    count++;
  }
  test_case("queue_foreach count", count == 3);

  queue_free(q);

  return !test_result;
}

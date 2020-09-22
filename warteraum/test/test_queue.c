#include <stdbool.h>
#include <string.h>
#include "test.h"
#include "../queue.h"

int main(void) {
  struct queue q;
  queue_new(&q);
  test_result = true;

  queue_append(&q, "test", 5);     // id: 0
  test_case("last setup correctly", q.last == q.first);
  test_case("last first ids", q.first->id == q.last->id);

  queue_append(&q, "hello", 6);    // id: 1
  queue_append(&q, "bizarre", 8);  // id: 2
  test_case("last id after additions", q.last->id == 2);

  queue_append(&q, "world", 6);    // id: 3
  queue_append(&q, "!", 2);        // id: 4
  test_case("last content after additions", strcmp("!", q.last->string) == 0);

  test_case("id of queue head", q.first->id == 0);

  queue_pop(&q);
  test_case("id after pop", q.first->id == 1);
  test_case("content after pop", strcmp(q.first->string, "hello") == 0);

  queue_remove(&q, 2);
  test_case("id of queue head intact after remove", q.first->id == 1);

  queue_pop(&q);
  test_case("id after pop", q.first->id == 3);
  test_case("content after pop", strcmp(q.first->string, "world") == 0);

  queue_append(&q, "stranger", 9); // id: 5

  unsigned int count = 0;
  bool ascending_ids = true;
  int last_id = -1;
  queue_foreach(q, elem) {
    count++;
    ascending_ids = ascending_ids &&
      (last_id < (int) elem->id);
  }
  test_case("queue_foreach count", count == 3);
  test_case("ids are ascending", ascending_ids);

  queue_remove(&q, 1312);
  count = 0;
  queue_foreach(q, elem) {
    count++;
  }
  test_case("bogus remove", count == 3);

  queue_pop(&q);
  queue_remove(&q, 5);
  test_case("remove last works properly", q.last->id == 4);

  queue_pop(&q);
  test_case("final pop works properly", q.first == NULL && q.last == NULL);

  // should not segfault
  queue_remove(&q, 9000);

  // some bogus values so we can check queue_free using valgrind
  queue_append(&q, "lol", 4);
  queue_append(&q, "foo", 4);
  queue_append(&q, "bar", 4);
  queue_append(&q, "baz", 4);

  queue_free(q);

  return !test_result;
}

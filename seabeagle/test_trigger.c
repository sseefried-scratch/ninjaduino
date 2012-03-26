#include "trigger.h"

void acts_like_falls_below(triggerfunction tf) {
  triggermemory_t mem;
  assert(tf!=NULL);
  init_memory(&mem, 100, 400);

  int result;

  result = tf(&mem, 200);
  assert(!result);
  result = tf(&mem, 100);
  assert(result);
  result = tf(&mem, 100);
  assert(!result);
  result = tf(&mem, 500);
  assert(!result);
  result = tf(&mem, 100);
  assert(result);
}

void acts_like_rises_above(triggerfunction tf) {
  triggermemory_t mem;
  assert(tf!=NULL);
  init_memory(&mem, 300, 100);

  int result;

  result = tf(&mem, 200);
  assert(!result);
  result = tf(&mem, 300);
  assert(result);
  result = tf(&mem, 300);
  assert(!result);
  result = tf(&mem, 50);
  assert(!result);
  result = tf(&mem, 300);
  assert(result);
}


int test_trigger() {
  acts_like_falls_below(find_trigger("light", "light_level_falls_below"));
  acts_like_rises_above(find_trigger("light", "light_level_rises_above"));

  acts_like_falls_below(find_trigger("distance", "distance_falls_below"));
  acts_like_rises_above(find_trigger("distance", "distance_rises_above"));
  return 0;
}

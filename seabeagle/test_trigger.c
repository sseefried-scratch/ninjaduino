#include "trigger.h"
main() {
  triggerfunction tf = find_trigger("light", "light_level_falls_below");
  assert(tf!=NULL);
  triggermemory_t mem;
  // default setup
  mem.trigger_level = 100;
  mem.reset_level = 400;
  mem.ready = 1;
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

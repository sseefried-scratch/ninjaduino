#include "cJSON.h"
#include <assert.h>
// reminder: json needs doublequotes.

int test_json() {
  char * simple = "{}";
  cJSON * root;

  root = cJSON_Parse(simple);
  assert(root);
  char * input = "{\"ports\": [{\"type\": \"DISTANCE\",\
  \"value\": 12,\
  \"port\": 1\
 }]}";
  root = cJSON_Parse(input);
  assert(root);
  
  
}

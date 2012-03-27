#include "config.h"
#include "assert.h"
#include <sqlite3.h>

int same_config(config_t * desired, config_t * found) {
  assert(strcmp(desired->identity, found->identity ) == 0);
  assert(strcmp(desired->broker_endpoint, found->broker_endpoint ) == 0);
  assert(strcmp(desired->portwatcher_endpoint, found->portwatcher_endpoint ) == 0);
}

int test_db() {
  sqlite3 * db;
  config_t result;
  config_t desired = {"8888", "tcp://localhost:9999", "inproc://happy"};
  printf("testing db...\n");
  unlink("/tmp/tmp.db");
  assert(0==sqlite3_open("/tmp/tmp.db", &db));
  
  assert(0!=record_config(db, &desired));
  assert(0!=retrieve_config(db, &desired));
  assert(0==create_db(db));
  assert(0==record_config(db, &desired));
  assert(0==retrieve_config(db, &result));
  same_config(&desired, &result);
  printf("test_db passed!\n");
  unlink("/tmp/tmp.db");
  return 0;
}

int test_config_case(char * input, config_t * desired_config) {
  FILE * f = fmemopen(input, strlen(input), "r");
  config_t found;

  parse_config(&found, f);
  same_config(desired_config, &found);
}

int test_config() {
  config_t c;
  // FIX currently doesn't work unless there's a trailing newline
  config_t desired = {"8888", "tcp://localhost:9999", "inproc://happy"};
  test_config_case("broker: tcp://localhost:9999\n\
portwatcher: inproc://happy\n\
identity: 8888\n", &desired);
  
  // bad case - no trailing newline
   test_config_case("broker: tcp://localhost:9999\n\
portwatcher: inproc://happy\n\
identity: 8888", &desired);

   // should ignore trailing & leading whitespace.
  
}

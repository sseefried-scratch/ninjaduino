#include "trigger.c"
#include <msgpack.h>

main() {
  char * literal_msgpack = "\x83\xA7line_id\f\xADtrigger_levelP\xABreset_levelQ"; 


  msgpack_sbuffer * buffer = msgpack_sbuffer_new();
  msgpack_sbuffer_write(buffer, literal_msgpack, strlen(literal_msgpack));

  /* deserializes it. */
  msgpack_unpacked msg;
  msgpack_unpacked_init(&msg);
  bool success = msgpack_unpack_next(&msg, buffer->data, buffer->size, NULL);
  trigger_t trigger;
  int rc = parse_trigger(&(msg.data), &trigger);
  assert(rc);

  assert(trigger.line_id == 12);
  assert(trigger.trigger_level == 80);
  assert(trigger.reset_level == 81);
  
  /* cleaning */
  msgpack_sbuffer_free(buffer);
  return(0);
 
}

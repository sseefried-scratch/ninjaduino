#include "trigger.c"
#include <msgpack.h>

main() {
  // {line_id:12, trigger_level:80, reset_level:81}
  char * literal_msgpack = "\x83\xA7line_id\f\xADtrigger_levelP\xABreset_levelQ"; 


  msgpack_sbuffer * buffer = msgpack_sbuffer_new();
  msgpack_sbuffer_write(buffer, literal_msgpack, strlen(literal_msgpack));

  /* deserializes it. */
  msgpack_unpacked msg;
  msgpack_unpacked_init(&msg);
  bool success = msgpack_unpack_next(&msg, buffer->data, buffer->size, NULL);
  assert(success);
  trigger_t trigger;
  int rc = parse_trigger(&(msg.data), &trigger);
  assert(rc);

  assert(trigger.line_id == 12);
  assert(trigger.trigger_level == 80);
  assert(trigger.reset_level == 81);
  char * minimal = "\x81\xA7line_id\v";
  msgpack_sbuffer_clear(buffer);
  msgpack_sbuffer_write(buffer, minimal, strlen(minimal));
  success = msgpack_unpack_next(&msg, buffer->data, buffer->size, NULL);
  assert(success);
  rc = parse_trigger(&(msg.data), &trigger);
  assert(rc);
  assert(trigger.line_id == 12);
  // some default values for the others
  // how does it find it? TODO
  msgpack_unpacked_init(&msg);
  char * invalid =  "\x80";


  msgpack_sbuffer_clear(buffer);
  msgpack_sbuffer_write(buffer, invalid, strlen(invalid));
  success = msgpack_unpack_next(&msg, buffer->data, buffer->size, NULL);
  assert(success);
  rc = parse_trigger(&(msg.data), &trigger);
  assert(!rc);



  /* cleaning */
  msgpack_sbuffer_free(buffer);
  return(0);
 
}

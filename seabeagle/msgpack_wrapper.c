/* the msgpack interface is a nightmare, so let's quarantine it */
#include <msgpack.h>
#include <czmq.h>
msgpack_object * parse_msgpack(msgpack_zone * mempool,
                               char * frame) {
  msgpack_object * deserialised = malloc(sizeof(msgpack_object));

  msgpack_unpack (frame, 
                  strlen(frame),
                  NULL, mempool, deserialised); 
  return deserialised;
}

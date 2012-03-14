/* the msgpack interface is a nightmare, so let's quarantine it */
#include <msgpack.h>
#include <czmq.h>
msgpack_object * parse_msgpack(msgpack_zone * mempool,
                               zframe_t * frame) {
  msgpack_object * deserialised = malloc(sizeof(msgpack_object));

  msgpack_unpack (zframe_data(frame), 
                  zframe_size(frame),
                  NULL, mempool, deserialised); 
  return deserialised;
}

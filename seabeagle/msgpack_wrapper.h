
#ifndef __MSGPACKWRAPPER_H__
#define __MSGPACKWRAPPER_H__
#include <czmq.h>
#include <msgpack.h>

msgpack_object * parse_msgpack(msgpack_zone * mempool, zframe_t * frame);

#endif

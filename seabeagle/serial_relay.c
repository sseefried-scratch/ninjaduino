#include "zhelpers.h"
#include <czmq.h>
#include "utils.h"
#include "serial_relay.h"

void read_serial(void * cvoid, zctx_t * context, void * pipe) {
  char * buf;
  serialconfig_t * config = (serialconfig_t *) cvoid;
  
  size_t nbytes=2047;
  //  Prepare our context and publisher
  buf = (char *) malloc(nbytes+1) ;
  fprintf(stderr, "bound\n");
  // first line is always garbage
  getline(&buf, &nbytes, config->in);
  child_handshake(pipe);
  zsocket_destroy(context, pipe);
  void* socket = zsocket_new(context, ZMQ_PUB);
  zsocket_bind(socket, "inproc://raw_serial");
  
  while ( getline(&buf, &nbytes, config->in) != -1 ) {
#ifdef DEBUG
    puts(buf);
#endif
    s_send (socket, buf);
  }
  fprintf(stderr, "error reading from stdin\n");
  zsocket_destroy(context, socket);
}

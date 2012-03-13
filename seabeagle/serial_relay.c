#include <czmq.h>
#include "serial_relay.h"
#include "utils.h"
#include <stdio.h>

void read_serial(void * cvoid, zctx_t * context, void * pipe) {
  char * buf;
  serialconfig_t * config = (serialconfig_t*)cvoid;
  
  FILE * in = config->in; // fopen("/dev/ttyO1", "r");

  size_t nbytes=2047;
  //  Prepare our context and publisher
  buf = (char *) malloc(nbytes+1) ;
  fprintf(stderr, "bound\n");
  // first line is always garbage
  getline(&buf, &nbytes, in);
  child_handshake(pipe);
  zsocket_destroy(context, pipe);
  void* socket = zsocket_new(context, ZMQ_PUB);
  zsocket_bind(socket, "inproc://raw_serial");
  
  while ( getline(&buf, &nbytes, in) != -1 ) {
#ifdef DEBUG
    puts("line:");
    puts(buf);

#endif
    zmsg_t * msg = zmsg_new();
    zmsg_pushstr(msg, buf); // does buf need to be copied?
    zmsg_send(&msg, socket);
  }
  fprintf(stderr, "error reading from stdin\n");
  zsocket_destroy(context, socket);
}

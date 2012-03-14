#include <czmq.h>
#include "utils.h"

void child_handshake(void*pipe) {
  recv_sync("ping", pipe);
  send_sync("pong", pipe);
}


void parent_handshake(void*pipe) {
  send_sync("ping", pipe);
  recv_sync("pong", pipe);
}

void recv_sync(char * expected, void * pipe) {
  assert(pipe);
  assert(expected);
  zmsg_t * in = zmsg_recv(pipe);
  if(!in) {
    zclock_log("quitting in recv_sync handshake!");
    exit(1);
  }
  assert(zmsg_size(in) == 1);
  zframe_t* ping = zmsg_pop(in);
  assert(zframe_streq(ping, expected));
  zmsg_destroy(&in);
}

void send_sync(char * msg, void * pipe) {
  assert(msg);
  assert(pipe);
  zmsg_t * out = zmsg_new();
  zmsg_pushstr(out, msg);
  zmsg_send(&out, pipe);
}



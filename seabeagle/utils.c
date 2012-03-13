#include <czmq.h>

void child_handshake(void*pipe) {
  zmsg_t * in = zmsg_recv(pipe);
  assert(zmsg_size(in) == 1);
  zframe_t* ping = zmsg_pop(in);
  assert(zframe_streq(ping, "ping"));
  zmsg_destroy(&in);
  zframe_destroy(&ping);
  zmsg_t * out = zmsg_new();
  zmsg_pushstr(out, "pong");
}


void parent_handshake(void*pipe) {
  
  zmsg_t * out = zmsg_new();
  zmsg_pushstr(out, "ping");
  zmsg_send(pipe,out);

  zmsg_t * in = zmsg_recv(pipe);
  assert(zmsg_size(in) == 1);
  zframe_t* pong = zmsg_pop(in);
  assert(zframe_streq(pong, "pong"));
  zmsg_destroy(&in);
  zframe_destroy(&pong);

}


